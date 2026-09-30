/*
  Prueba MPU6050 + ESP32-S3 con calculo de ANGULO
  Pines segun el esquematico final de la PCB.

  Conexiones:
    VCC -> 3V3      GND -> GND
    SDA -> GPIO 48
    SCL -> GPIO 47
    INT -> GPIO 21  (no se usa en esta prueba)

  Muestra tres columnas:
    ACC   -> angulo calculado solo del acelerometro (ruidoso, sin deriva)
    GYRO  -> angulo integrado del giroscopio (suave, con deriva)
    FILT  -> filtro complementario (el que sirve para el control)

  Comando (Monitor Serie, 115200):
    'c' -> recalibrar el offset del giroscopio (dejar el sensor QUIETO)
*/

#include <Wire.h>
#include <Adafruit_MPU6050.h>
#include <Adafruit_Sensor.h>

#define SDA_PIN 48
#define SCL_PIN 47

Adafruit_MPU6050 mpu;

float anguloGyro = 0.0;      // angulo integrado del giroscopio
float anguloFiltrado = 0.0;  // salida del filtro complementario
float offsetGyro = 0.0;      // deriva en reposo, se mide al calibrar

uint32_t tiempoAnterior = 0;

// Peso del giroscopio en el filtro. Mas alto = mas suave pero corrige
// la deriva mas lento. Valor tipico entre 0.96 y 0.99.
const float ALFA = 0.98;

void calibrarGiroscopio() {
  Serial.println();
  Serial.println(F(">> Calibrando giroscopio. MANTEN EL SENSOR QUIETO..."));
  delay(1000);

  float suma = 0;
  const int MUESTRAS = 500;

  for (int i = 0; i < MUESTRAS; i++) {
    sensors_event_t a, g, t;
    mpu.getEvent(&a, &g, &t);
    suma += g.gyro.x;          // eje de inclinacion, ver nota abajo
    delay(3);
  }

  offsetGyro = suma / MUESTRAS;
  Serial.printf(">> Offset del giroscopio: %.5f rad/s\n", offsetGyro);
  Serial.println();
}

void setup() {
  Serial.begin(115200);
  delay(2000);

  Wire.begin(SDA_PIN, SCL_PIN);

  if (!mpu.begin()) {
    Serial.println(F("No se detecto el MPU6050."));
    Serial.println(F("Revisa alimentacion 3.3V y cableado SDA/SCL."));
    while (1) delay(1000);
  }

  Serial.println(F("MPU6050 detectado"));

  mpu.setAccelerometerRange(MPU6050_RANGE_4_G);
  mpu.setGyroRange(MPU6050_RANGE_500_DEG);
  mpu.setFilterBandwidth(MPU6050_BAND_44_HZ);

  calibrarGiroscopio();

  // Inicializa el angulo con la lectura del acelerometro
  sensors_event_t a, g, t;
  mpu.getEvent(&a, &g, &t);
  anguloFiltrado = atan2(a.acceleration.y, a.acceleration.z) * 180.0 / PI;
  anguloGyro = anguloFiltrado;

  tiempoAnterior = micros();

  Serial.println(F("Comando: 'c' para recalibrar"));
  Serial.println();
  Serial.println(F("ACC\tGYRO\tFILT"));
}

void loop() {
  while (Serial.available()) {
    char c = Serial.read();
    if (c == 'c') {
      calibrarGiroscopio();
      anguloGyro = anguloFiltrado;
    }
  }

  sensors_event_t a, g, t;
  mpu.getEvent(&a, &g, &t);

  // Tiempo transcurrido desde la lectura anterior, en segundos
  uint32_t ahora = micros();
  float dt = (ahora - tiempoAnterior) / 1000000.0;
  tiempoAnterior = ahora;

  // Angulo desde el acelerometro (grados)
  float anguloAcc = atan2(a.acceleration.y, a.acceleration.z) * 180.0 / PI;

  // Velocidad angular corregida por el offset, pasada a grados/s
  float velAngular = (g.gyro.x - offsetGyro) * 180.0 / PI;

  // Integracion pura del giroscopio (para ver la deriva)
  anguloGyro += velAngular * dt;

  // Filtro complementario
  anguloFiltrado = ALFA * (anguloFiltrado + velAngular * dt)
                 + (1.0 - ALFA) * anguloAcc;

  // Imprime cada 100 ms para no saturar el monitor
  static uint32_t ultimaImpresion = 0;
  if (millis() - ultimaImpresion >= 100) {
    ultimaImpresion = millis();
    Serial.printf("%.1f\t%.1f\t%.1f\n", anguloAcc, anguloGyro, anguloFiltrado);
  }
}