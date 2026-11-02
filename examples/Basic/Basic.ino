// AlashLogger: запись показаний в CSV на SD-карту (откроется в Excel и Google Таблицах).
// SD-модуль: Uno: CS - 10, MOSI - 11, MISO - 12, SCK - 13, VCC 5 В. ESP32: CS - 5, MOSI - 23, MISO - 19, SCK - 18.
#include <AlashLogger.h>

#if defined(ESP32)
const uint8_t CS_PIN = 5;
#elif defined(ESP8266)
const uint8_t CS_PIN = 15;
#else
const uint8_t CS_PIN = 10;
#endif

AlashLogger logger;

void setup() {
  Serial.begin(115200);
  logger.setColumns("signal,volts");            // заголовок: time,signal,volts
  if (!logger.begin(CS_PIN, "data")) {          // файл data.csv
    Serial.println(logger.errorText());
    while (true) delay(1000);
  }
}

void loop() {
  int raw = analogRead(A0);
  logger.add(raw);
  logger.add(raw * 5.0f / 1023.0f, 2);          // 2 знака после запятой
  if (!logger.save()) Serial.println(logger.errorText());
  delay(1000);
}
