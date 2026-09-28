/*
  PRUEBA TB6612FNG - dos motores 25GA370 + ESP32-S3
  Version para core ESP32 3.x (API LEDC nueva)

  Conexiones:
    VM   -> +12V fuente        VCC  -> 3V3 de la ESP32
    GND  -> GND comun (fuente + ESP32)
    STBY -> GPIO 10
    PWMA -> GPIO 9    AIN1 -> GPIO 46   AIN2 -> GPIO 3
    PWMB -> GPIO 21   BIN1 -> GPIO 47   BIN2 -> GPIO 48
    AO1/AO2 -> motor 1 (rojo y blanco)
    BO1/BO2 -> motor 2 (rojo y blanco)

  Comandos (Monitor Serie, 115200):
    'a' -> motor A adelante     'z' -> motor A atras
    's' -> motor B adelante     'x' -> motor B atras
    'd' -> ambos adelante       'c' -> ambos atras
    ' ' -> parar todo
    '+' / '-' -> subir o bajar la velocidad
*/

const uint8_t PIN_STBY = 10;
const uint8_t PIN_PWMA = 9;
const uint8_t PIN_AIN1 = 1;
const uint8_t PIN_AIN2 = 2;
const uint8_t PIN_PWMB = 21;
const uint8_t PIN_BIN1 = 47;
const uint8_t PIN_BIN2 = 48;

const uint32_t FRECUENCIA_PWM = 20000;   // 20 kHz, fuera del rango audible
const uint8_t RESOLUCION_PWM = 8;        // 0 .. 255

int velocidad = 120;   // valor inicial, moderado

// direccion: 1 adelante, -1 atras, 0 parar
void motorA(int direccion, int vel) {
  if (direccion == 1)       { digitalWrite(PIN_AIN1, HIGH); digitalWrite(PIN_AIN2, LOW);  }
  else if (direccion == -1) { digitalWrite(PIN_AIN1, LOW);  digitalWrite(PIN_AIN2, HIGH); }
  else                      { digitalWrite(PIN_AIN1, LOW);  digitalWrite(PIN_AIN2, LOW);  vel = 0; }
  ledcWrite(PIN_PWMA, vel);        // en el core 3.x se pasa el PIN, no el canal
}

void motorB(int direccion, int vel) {
  if (direccion == 1)       { digitalWrite(PIN_BIN1, HIGH); digitalWrite(PIN_BIN2, LOW);  }
  else if (direccion == -1) { digitalWrite(PIN_BIN1, LOW);  digitalWrite(PIN_BIN2, HIGH); }
  else                      { digitalWrite(PIN_BIN1, LOW);  digitalWrite(PIN_BIN2, LOW);  vel = 0; }
  ledcWrite(PIN_PWMB, vel);
}

void pararTodo() {
  motorA(0, 0);
  motorB(0, 0);
}

void setup() {
  Serial.begin(115200);
  delay(2000);

  pinMode(PIN_STBY, OUTPUT);
  pinMode(PIN_AIN1, OUTPUT);
  pinMode(PIN_AIN2, OUTPUT);
  pinMode(PIN_BIN1, OUTPUT);
  pinMode(PIN_BIN2, OUTPUT);

  // API nueva: una sola llamada por pin
  ledcAttach(PIN_PWMA, FRECUENCIA_PWM, RESOLUCION_PWM);
  ledcAttach(PIN_PWMB, FRECUENCIA_PWM, RESOLUCION_PWM);

  digitalWrite(PIN_STBY, HIGH);   // habilita el driver
  pararTodo();

  Serial.println(F("PRUEBA TB6612FNG"));
  Serial.println(F("a/z: motor A adelante/atras"));
  Serial.println(F("s/x: motor B adelante/atras"));
  Serial.println(F("d/c: ambos adelante/atras"));
  Serial.println(F("espacio: parar   +/-: velocidad"));
  Serial.printf("Velocidad inicial: %d\n", velocidad);
}

void loop() {
  while (Serial.available()) {
    char c = Serial.read();

    switch (c) {
      case 'a': motorA(1, velocidad);  Serial.println(F(">> A adelante")); break;
      case 'z': motorA(-1, velocidad); Serial.println(F(">> A atras"));    break;
      case 's': motorB(1, velocidad);  Serial.println(F(">> B adelante")); break;
      case 'x': motorB(-1, velocidad); Serial.println(F(">> B atras"));    break;
      case 'd': motorA(1, velocidad);  motorB(1, velocidad);
                Serial.println(F(">> Ambos adelante")); break;
      case 'c': motorA(-1, velocidad); motorB(-1, velocidad);
                Serial.println(F(">> Ambos atras")); break;
      case ' ': pararTodo(); Serial.println(F(">> PARADO")); break;
      case '+': velocidad += 20; if (velocidad > 255) velocidad = 255;
                Serial.printf(">> Velocidad: %d\n", velocidad); break;
      case '-': velocidad -= 20; if (velocidad < 0) velocidad = 0;
                Serial.printf(">> Velocidad: %d\n", velocidad); break;
    }
  }

  delay(10);
}