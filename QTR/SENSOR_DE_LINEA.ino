/*
  Identificacion de la placa roja: ¿analogica o RC?
  Lee los 8 pines con analogRead, sin metodo RC.

  IR -> GPIO 8   |   D1..D8 -> GPIO 4, 5, 6, 7, 15, 16, 17, 18   |   VCC -> 3V3
  (15-18 son ADC2: funcionan porque no usamos WiFi en esta prueba)

  Comandos (Monitor Serie, 115200):  '1' emisores ON   '0' emisores OFF
*/

const uint8_t NUM_SENSORES = 8;
const uint8_t PINES_SENSOR[NUM_SENSORES] = {4, 5, 6, 7, 15, 16, 17, 18};
const uint8_t PIN_IR = 8;

bool emisoresEncendidos = true;
uint16_t lineasImpresas = 0;

void setup() {
  Serial.begin(115200);
  delay(2000);

  pinMode(PIN_IR, OUTPUT);
  digitalWrite(PIN_IR, HIGH);

  for (uint8_t i = 0; i < NUM_SENSORES; i++) {
    pinMode(PINES_SENSOR[i], INPUT);
  }

  analogReadResolution(12);   // 0 .. 4095

  Serial.println(F("IDENTIFICACION PLACA ROJA (lectura analogica)"));
  Serial.println(F("Comandos: '1' ON   '0' OFF"));
}

void loop() {
  while (Serial.available()) {
    char c = Serial.read();
    if (c == '1') { digitalWrite(PIN_IR, HIGH); emisoresEncendidos = true;
                    Serial.println(F(">> Emisores ON"));  lineasImpresas = 0; }
    if (c == '0') { digitalWrite(PIN_IR, LOW);  emisoresEncendidos = false;
                    Serial.println(F(">> Emisores OFF")); lineasImpresas = 0; }
  }

  if (lineasImpresas % 20 == 0) {
    Serial.println(F("IR\tD1\tD2\tD3\tD4\tD5\tD6\tD7\tD8"));
  }
  lineasImpresas++;

  Serial.print(emisoresEncendidos ? F("ON") : F("OFF"));
  for (uint8_t i = 0; i < NUM_SENSORES; i++) {
    Serial.print('\t');
    Serial.print(analogRead(PINES_SENSOR[i]));
  }
  Serial.println();

  delay(300);
}