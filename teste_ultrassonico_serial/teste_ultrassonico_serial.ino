
/*
 * Teste isolado do HC-SR04 para ESP32-S3.
 *
 * Ligacao usada no firmware do MinionRobo:
 *   TRIG -> GPIO 17
 *   ECHO -> GPIO 18
 *
 * Este programa nao usa Wi-Fi e nao controla motores.
 */

constexpr uint8_t PIN_TRIG = 17;
constexpr uint8_t PIN_ECHO = 18;
constexpr unsigned long TIMEOUT_ECHO_US = 30000UL;

void setup() {
  Serial.begin(115200);
  delay(1000);

  pinMode(PIN_TRIG, OUTPUT);
  pinMode(PIN_ECHO, INPUT);
  digitalWrite(PIN_TRIG, LOW);

  Serial.println();
  Serial.println(F("=== TESTE HC-SR04 ==="));
  Serial.println(F("TRIG: GPIO 17 | ECHO: GPIO 18"));
  Serial.println(F("Aproxime um objeto a 10-100 cm."));
  Serial.println(F("ECHO precisa chegar ao ESP32 com no maximo 3,3 V."));
  Serial.println();
}

void loop() {
  // Pulso de disparo do HC-SR04: 10 us em nivel alto.
  digitalWrite(PIN_TRIG, LOW);
  delayMicroseconds(2);
  digitalWrite(PIN_TRIG, HIGH);
  delayMicroseconds(10);
  digitalWrite(PIN_TRIG, LOW);

  const unsigned long duracaoUs = pulseIn(
    PIN_ECHO,
    HIGH,
    TIMEOUT_ECHO_US
  );

  Serial.print(F("ECHO="));
  Serial.print(digitalRead(PIN_ECHO));
  Serial.print(F(" | pulso="));
  Serial.print(duracaoUs);
  Serial.print(F(" us | "));

  if (duracaoUs == 0) {
    Serial.println(F("SEM ECO - verifique VCC, GND, TRIG e ECHO"));
  } else {
    const float distanciaCm = duracaoUs * 0.0343f / 2.0f;
    Serial.print(F("distancia="));
    Serial.print(distanciaCm, 1);
    Serial.println(F(" cm"));
  }

  delay(500);
}
