// AlashLogger: запись на SD не работает? Скетч проверит карту, создаст пробный файл и подскажет, что проверить.
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
  delay(500);
  logger.setLanguage(AlashLogger::LANG_RU);   // LANG_KK: қазақша, LANG_EN: English
  logger.begin(CS_PIN, "diag");
  logger.diagnose(Serial);
}

void loop() {}
