// AlashLogger: когда файл вырастает до 20 КБ, запись продолжается в новом файле (data.csv, data_1.csv, ...).
// После перезапуска плата продолжает в последнем файле, ничего не стирается.
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
  logger.setColumns("value");
  logger.setMaxFileSize(20000UL);
  logger.setFlushEvery(2000);                   // сбрасывать на карту раз в 2 секунды
  if (!logger.begin(CS_PIN, "data")) {
    Serial.println(logger.errorText());
    while (true) delay(1000);
  }
}

void loop() {
  logger.add((long)random(0, 1000));
  logger.save();
  if (logger.rows() % 20 == 0) { Serial.print(logger.fileName()); Serial.print(' '); Serial.println(logger.fileSize()); }
  delay(100);
}
