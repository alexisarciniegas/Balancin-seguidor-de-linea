/*
  LAZO DE EQUILIBRIO EN CASCADA - control por botones
    Lazo interno (200 Hz):  PD de angulo      -> PWM
    Lazo externo (20 Hz):   PI de velocidad   -> ajuste del setpoint de angulo

  ESTADO: el lazo externo esta MONTADO pero INACTIVO (Kp_vel = Ki_vel = 0).
  Esta version sirve para verificar el signo de la velocidad por el monitor.

  Pines:
    MPU6050  SDA 48, SCL 47
    TB6612   PWMA 4, AIN2 5, AIN1 6, STBY 7, BIN1 15, BIN2 16, PWMB 17
    Encoder A: 1 (canal A), 2 (canal B)
    Encoder B: 42 (canal A), 41 (canal B)
    Boton CALIBRAR 40, Boton START 39, LED 45   (pull-up: LOW = presionado)

  Botones:
    CALIBRAR corto -> captura el PUNTO DE EQUILIBRIO promediando
    CALIBRAR largo -> invierte el signo de la correccion de angulo
    START          -> activa / desactiva el control

  LED:
    apagado          -> sin punto de equilibrio capturado
    parpadeo lento   -> listo, control apagado
    encendido fijo   -> control ACTIVO (o promediando durante la captura)
    parpadeo rapido  -> se cayo. Presiona START para rearmar.
  Confirmacion de captura:
    2 parpadeos lentos  -> captura BUENA
    4 parpadeos rapidos -> captura INESTABLE, repitela

  Monitor serie (115200): imprime ANG, RPM y los dos contadores de pulsos,
  SOLO con el control apagado, para no estorbar al lazo.
*/

#include <Wire.h>
#include <Adafruit_MPU6050.h>
#include <Adafruit_Sensor.h>

// ---------------- Pines ----------------
const uint8_t SDA_PIN = 48, SCL_PIN = 47;
const uint8_t PIN_PWMA = 4,  PIN_AIN2 = 5,  PIN_AIN1 = 6,  PIN_STBY = 7;
const uint8_t PIN_BIN1 = 15, PIN_BIN2 = 16, PIN_PWMB = 17;
const uint8_t PIN_BTN_CAL = 40, PIN_BTN_START = 39, PIN_LED = 45;
const uint8_t ENC_A_CH1 = 1,  ENC_A_CH2 = 2;     // encoder motor A
const uint8_t ENC_B_CH1 = 42, ENC_B_CH2 = 41;    // encoder motor B

// ---------------- PWM ----------------
const uint32_t FREC_PWM = 20000;
const uint8_t  RES_PWM  = 8;

// ---------------- Lazo interno: angulo (PD) ----------------
float Kp = 39;
float Kd = 1.5;
float Ki = 0.0;             // el lazo interno va sin integral

const int   PWM_MAX     = 200;
const int   PWM_MINIMO  = 12;
const float LIMITE_CAIDA = 35.0;
const float I_MAX       = 400.0;

const uint16_t PERIODO_US = 5000;   // 5 ms -> 200 Hz

// ---------------- Lazo externo: velocidad (PI) ----------------
const float PULSOS_POR_VUELTA = 822.5;   // MEDIDO sobre 10 vueltas


float Kp_vel = 0.09;
float Ki_vel = 0.1;    // sigue en cero

float velocidadObjetivo = 0.0;   // RPM. Sera distinto de cero al seguir linea

const uint8_t DIVISOR_EXTERNO = 10;      // 10 x 5 ms = 50 ms -> 20 Hz
const float   LIMITE_AJUSTE   = 6.0;     // grados: tope del ajuste
const float   I_MAX_VEL       = 150.0;   // anti-windup del integrador
const float FILTRO_VEL = 1;   // 0 = muy suave, 1 = sin filtro

// VERIFICAR CON EL ROBOT (paso 5). Si empeora, poner -1.
int signoVelocidad = 1;

// Signos de conteo. VERIFICADO: rodando hacia adelante el encoder A
// cuenta negativo y el B positivo, por eso el A va invertido.
int signoEncoderA =  -1;
int signoEncoderB =  1;

// ---------------- Captura del punto de equilibrio ----------------
const uint16_t MS_ASENTAMIENTO = 300;
const uint16_t MS_PROMEDIO     = 3700;
const float    DISPERSION_MAX  = 2.0;

// ---------------- Filtro ----------------
const float ALFA = 0.98;

Adafruit_MPU6050 mpu;

float anguloFiltrado = 0.0;
float offsetGyro = 0.0;
float puntoEquilibrio = 0.0;
float integral = 0.0;
int   signoCorreccion = -1;

bool equilibrioCapturado = false;
bool pidActivo = false;
bool seCayo = false;

uint32_t tAnterior = 0;
uint32_t tCiclo = 0;

// Estado del lazo externo
volatile long pulsosA = 0;
volatile long pulsosB = 0;
long  pulsosA_prev = 0, pulsosB_prev = 0;
uint8_t contadorExterno = 0;
float velocidadRPM   = 0.0;
float integralVel    = 0.0;
float ajusteSetpoint = 0.0;

// ================= Encoders =================
void IRAM_ATTR isrEncoderA() {
  if (digitalRead(ENC_A_CH2) == HIGH) pulsosA++;
  else                                pulsosA--;
}

void IRAM_ATTR isrEncoderB() {
  if (digitalRead(ENC_B_CH2) == HIGH) pulsosB++;
  else                                pulsosB--;
}

void reiniciarEncoders() {
  noInterrupts();
  pulsosA = 0;
  pulsosB = 0;
  interrupts();
  pulsosA_prev    = 0;
  pulsosB_prev    = 0;
  contadorExterno = 0;
  velocidadRPM    = 0.0;
  integralVel     = 0.0;
  ajusteSetpoint  = 0.0;
}

// ================= Motores =================
void motorA(int dir, int vel) {
  if (dir > 0)      { digitalWrite(PIN_AIN1, HIGH); digitalWrite(PIN_AIN2, LOW);  }
  else if (dir < 0) { digitalWrite(PIN_AIN1, LOW);  digitalWrite(PIN_AIN2, HIGH); }
  else              { digitalWrite(PIN_AIN1, LOW);  digitalWrite(PIN_AIN2, LOW); vel = 0; }
  ledcWrite(PIN_PWMA, vel);
}

void motorB(int dir, int vel) {
  if (dir > 0)      { digitalWrite(PIN_BIN1, HIGH); digitalWrite(PIN_BIN2, LOW);  }
  else if (dir < 0) { digitalWrite(PIN_BIN1, LOW);  digitalWrite(PIN_BIN2, HIGH); }
  else              { digitalWrite(PIN_BIN1, LOW);  digitalWrite(PIN_BIN2, LOW); vel = 0; }
  ledcWrite(PIN_PWMB, vel);
}

void parar() { motorA(0, 0); motorB(0, 0); }

void aplicarSalida(int salida) {
  int dir = (salida > 0) ? 1 : ((salida < 0) ? -1 : 0);
  int vel = abs(salida);
  if (vel > 0 && vel < PWM_MINIMO) vel = PWM_MINIMO;   // zona muerta
  if (vel > PWM_MAX) vel = PWM_MAX;
  motorA(dir, vel);
  motorB(dir, vel);
}

// ================= LED =================
void parpadear(uint8_t veces, uint16_t ms) {
  for (uint8_t i = 0; i < veces; i++) {
    digitalWrite(PIN_LED, HIGH); delay(ms);
    digitalWrite(PIN_LED, LOW);  delay(ms);
  }
}

void actualizarLed() {
  static uint32_t t = 0;
  static bool estado = false;
  uint16_t periodo;

  if (pidActivo)             { digitalWrite(PIN_LED, HIGH); return; }
  else if (seCayo)           periodo = 100;
  else if (equilibrioCapturado) periodo = 600;
  else                       { digitalWrite(PIN_LED, LOW); return; }

  if (millis() - t >= periodo) {
    t = millis();
    estado = !estado;
    digitalWrite(PIN_LED, estado);
  }
}

// ================= Lectura del sensor =================
float actualizarAngulo(float dt) {
  sensors_event_t a, g, t;
  mpu.getEvent(&a, &g, &t);

  float anguloAcc  = atan2(a.acceleration.y, a.acceleration.z) * 180.0 / PI;
  float velAngular = (g.gyro.x - offsetGyro) * 180.0 / PI;

  anguloFiltrado = ALFA * (anguloFiltrado + velAngular * dt)
                 + (1.0 - ALFA) * anguloAcc;

  return velAngular;
}

// ================= Giroscopio =================
void calibrarGiro() {
  float suma = 0;
  const int N = 500;
  for (int i = 0; i < N; i++) {
    sensors_event_t a, g, t;
    mpu.getEvent(&a, &g, &t);
    suma += g.gyro.x;
    delay(3);
  }
  offsetGyro = suma / N;
}

// ================= Captura del punto de equilibrio =================
void capturarEquilibrio() {
  parar();
  pidActivo = false;
  digitalWrite(PIN_LED, HIGH);

  uint32_t tPrev = micros();

  uint32_t t0 = millis();
  while (millis() - t0 < MS_ASENTAMIENTO) {
    uint32_t ahora = micros();
    float dt = (ahora - tPrev) / 1000000.0;
    tPrev = ahora;
    if (dt <= 0 || dt > 0.1) dt = 0.005;
    actualizarAngulo(dt);
    delay(2);
  }

  double suma = 0;
  uint32_t n = 0;
  float minimo =  10000.0;
  float maximo = -10000.0;

  t0 = millis();
  while (millis() - t0 < MS_PROMEDIO) {
    uint32_t ahora = micros();
    float dt = (ahora - tPrev) / 1000000.0;
    tPrev = ahora;
    if (dt <= 0 || dt > 0.1) dt = 0.005;

    actualizarAngulo(dt);

    suma += anguloFiltrado;
    n++;
    if (anguloFiltrado < minimo) minimo = anguloFiltrado;
    if (anguloFiltrado > maximo) maximo = anguloFiltrado;

    delay(2);
  }

  digitalWrite(PIN_LED, LOW);

  if (n > 0) {
    puntoEquilibrio = suma / n;
    equilibrioCapturado = true;
    integral = 0;
  }

  tAnterior = micros();
  tCiclo = micros();

  if ((maximo - minimo) > DISPERSION_MAX) parpadear(4, 100);   // inestable
  else                                    parpadear(2, 200);   // buena
}

// ================= Botones =================
uint8_t leerBoton(uint8_t pin) {
  if (digitalRead(pin) != LOW) return 0;
  delay(30);
  if (digitalRead(pin) != LOW) return 0;

  uint32_t t0 = millis();
  while (digitalRead(pin) == LOW) {
    if (millis() - t0 > 3000) break;
    delay(10);
  }
  return (millis() - t0 >= 1000) ? 2 : 1;
}

void setup() {
  Serial.begin(115200);

  pinMode(PIN_BTN_CAL,   INPUT_PULLUP);
  pinMode(PIN_BTN_START, INPUT_PULLUP);
  pinMode(PIN_LED, OUTPUT);

  pinMode(PIN_STBY, OUTPUT);
  pinMode(PIN_AIN1, OUTPUT); pinMode(PIN_AIN2, OUTPUT);
  pinMode(PIN_BIN1, OUTPUT); pinMode(PIN_BIN2, OUTPUT);
  ledcAttach(PIN_PWMA, FREC_PWM, RES_PWM);
  ledcAttach(PIN_PWMB, FREC_PWM, RES_PWM);
  digitalWrite(PIN_STBY, HIGH);
  parar();

  pinMode(ENC_A_CH1, INPUT_PULLUP); pinMode(ENC_A_CH2, INPUT_PULLUP);
  pinMode(ENC_B_CH1, INPUT_PULLUP); pinMode(ENC_B_CH2, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(ENC_A_CH1), isrEncoderA, RISING);
  attachInterrupt(digitalPinToInterrupt(ENC_B_CH1), isrEncoderB, RISING);

  Wire.begin(SDA_PIN, SCL_PIN);
  if (!mpu.begin()) {
    while (1) { parpadear(1, 80); }
  }
  mpu.setAccelerometerRange(MPU6050_RANGE_4_G);
  mpu.setGyroRange(MPU6050_RANGE_500_DEG);
  mpu.setFilterBandwidth(MPU6050_BAND_44_HZ);

  digitalWrite(PIN_LED, HIGH);
  calibrarGiro();
  digitalWrite(PIN_LED, LOW);

  sensors_event_t a, g, t;
  mpu.getEvent(&a, &g, &t);
  anguloFiltrado = atan2(a.acceleration.y, a.acceleration.z) * 180.0 / PI;

  parpadear(3, 150);
  tAnterior = micros();
  tCiclo = micros();
}

void loop() {
  // ---------- Botones ----------
  uint8_t bc = leerBoton(PIN_BTN_CAL);
  if (bc == 1) {
    capturarEquilibrio();
  } else if (bc == 2) {
    signoCorreccion = -signoCorreccion;
    pidActivo = false;
    parar();
    parpadear(signoCorreccion > 0 ? 1 : 2, 400);
    tAnterior = micros();
    tCiclo = micros();
  }

  if (leerBoton(PIN_BTN_START) == 1) {
    if (equilibrioCapturado) {
      pidActivo = !pidActivo;
      seCayo = false;
      integral = 0;
      reiniciarEncoders();          // imprescindible al arrancar
      if (!pidActivo) parar();
      tAnterior = micros();
      tCiclo = micros();
    }
  }

  // ---------- Ciclo de control a frecuencia fija ----------
  if (micros() - tCiclo < PERIODO_US) { actualizarLed(); return; }
  tCiclo = micros();

  uint32_t ahora = micros();
  float dt = (ahora - tAnterior) / 1000000.0;
  tAnterior = ahora;
  if (dt <= 0 || dt > 0.1) dt = 0.005;

  float velAngular = actualizarAngulo(dt);

  // ===== LAZO EXTERNO: la velocidad se calcula SIEMPRE =====
  // (tambien con el control apagado, para poder verificarla por el monitor)
  if (++contadorExterno >= DIVISOR_EXTERNO) {
    contadorExterno = 0;
    const float dtExt = (PERIODO_US / 1000000.0) * DIVISOR_EXTERNO;   // 0,05 s

    noInterrupts();
    long pA = pulsosA, pB = pulsosB;
    interrupts();

    long deltaPulsos = signoEncoderA * (pA - pulsosA_prev)
                     + signoEncoderB * (pB - pulsosB_prev);
    pulsosA_prev = pA;
    pulsosB_prev = pB;

    float rpmCruda = ((float)deltaPulsos / (2.0 * PULSOS_POR_VUELTA))
                   * (60.0 / dtExt);
    // Promedio exponencial: suaviza el salto de la cuenta de pulsos
    velocidadRPM = FILTRO_VEL * rpmCruda + (1.0 - FILTRO_VEL) * velocidadRPM;

    // El PI solo actua con el control encendido
    if (pidActivo) {
      float errorVel = velocidadObjetivo - velocidadRPM;
      integralVel += errorVel * dtExt;
      integralVel = constrain(integralVel, -I_MAX_VEL, I_MAX_VEL);
      ajusteSetpoint = (errorVel * Kp_vel) + (integralVel * Ki_vel);
      ajusteSetpoint = constrain(ajusteSetpoint, -LIMITE_AJUSTE, LIMITE_AJUSTE);
    }
  }

  if (pidActivo) {
    // ===== LAZO INTERNO: angulo (PD) =====
    float setpoint = puntoEquilibrio + signoVelocidad * ajusteSetpoint;
    float error = anguloFiltrado - setpoint;

    if (fabs(error) > LIMITE_CAIDA) {      // se cayo
      pidActivo = false;
      seCayo = true;
      parar();
      actualizarLed();
      return;
    }

    integral += error * dt;
    if (integral >  I_MAX / (Ki > 0 ? Ki : 1)) integral =  I_MAX / (Ki > 0 ? Ki : 1);
    if (integral < -I_MAX / (Ki > 0 ? Ki : 1)) integral = -I_MAX / (Ki > 0 ? Ki : 1);

    float salida = Kp * error + Kd * velAngular + Ki * integral;
    salida *= signoCorreccion;

    aplicarSalida((int)salida);

  } else {
    // ---------- Telemetria, solo con el control APAGADO ----------
    static uint32_t tImp = 0;
    if (millis() - tImp >= 200) {
      tImp = millis();
      noInterrupts();
      long pA = pulsosA, pB = pulsosB;
      interrupts();
      Serial.printf("ANG:%7.2f  RPM:%7.1f  PulsosA:%8ld  PulsosB:%8ld\n",
                    anguloFiltrado, velocidadRPM, pA, pB);
    }
  }

  actualizarLed();
}