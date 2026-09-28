/*
  PRUEBA DE ENCODERS - 2 motores 25GA370 + ESP32-S3
  Gira los ejes A MANO. No se necesitan los 12V ni el driver.

  Conexiones (segun la ficha del fabricante):
    AZUL   (Encoder +)  -> 3V3      (NO invertir con el negro)
    NEGRO  (Encoder -)  -> GND
    AMARILLO M1 (canal A) -> GPIO 11
    VERDE    M1 (canal B) -> GPIO 12
    AMARILLO M2 (canal A) -> GPIO 13
    VERDE    M2 (canal B) -> GPIO 14
    ROJO y BLANCO (motor): SIN CONECTAR

  Comando (Monitor Serie, 115200):
    'r' -> poner ambos contadores en cero
*/

const uint8_t M1_ENC_A = 11;
const uint8_t M1_ENC_B = 12;
const uint8_t M2_ENC_A = 13;
const uint8_t M2_ENC_B = 14;

volatile long conteoM1 = 0;
volatile long conteoM2 = 0;

// Interrupcion en flanco de subida del canal A.
// El canal B indica el sentido de giro.
void IRAM_ATTR isrM1() {
  if (digitalRead(M1_ENC_B) == HIGH) conteoM1++;
  else                               conteoM1--;
}

void IRAM_ATTR isrM2() {
  if (digitalRead(M2_ENC_B) == HIGH) conteoM2++;
  else                               conteoM2--;
}

void setup() {
  Serial.begin(115200);
  delay(2000);

  pinMode(M1_ENC_A, INPUT);
  pinMode(M1_ENC_B, INPUT);
  pinMode(M2_ENC_A, INPUT);
  pinMode(M2_ENC_B, INPUT);

  attachInterrupt(digitalPinToInterrupt(M1_ENC_A), isrM1, RISING);
  attachInterrupt(digitalPinToInterrupt(M2_ENC_A), isrM2, RISING);

  Serial.println(F("PRUEBA DE ENCODERS - gira los ejes con la mano"));
  Serial.println(F("Comando: 'r' para reiniciar los contadores"));
  Serial.println();
  Serial.println(F("M1\tM2"));
}

void loop() {
  while (Serial.available()) {
    char c = Serial.read();
    if (c == 'r') {
      noInterrupts();
      conteoM1 = 0;
      conteoM2 = 0;
      interrupts();
      Serial.println(F(">> Contadores en cero"));
    }
  }

  noInterrupts();
  long m1 = conteoM1;
  long m2 = conteoM2;
  interrupts();

  Serial.printf("%ld\t%ld\n", m1, m2);

  delay(200);
}