// [ru] AlashLogger на ESP32 с модулем SD на своих пинах SPI (вторая шина HSPI), когда основная занята экраном или радиомодулем.
// [kk] AlashLogger ESP32-де SD модулі өз SPI пиндерінде (екінші HSPI шинасы), негізгі шина экранмен не радиомодульмен бос болмағанда.
// [en] AlashLogger on ESP32 with the SD module on its own SPI pins (the second bus, HSPI), when the main bus is taken by a display or a radio.
#include <AlashLogger.h>

#if defined(ESP32)
const int PIN_SCK = 14, PIN_MISO = 12, PIN_MOSI = 13, PIN_CS = 15;
SPIClass spiSd(HSPI);
AlashLogger logger;

void setup() {
  Serial.begin(115200);
  spiSd.begin(PIN_SCK, PIN_MISO, PIN_MOSI, PIN_CS);
  logger.setColumns("value");
  if (!logger.begin(PIN_CS, spiSd, "log")) {      // шина передаётся в SD
    Serial.println(logger.errorText());
    while (true) delay(1000);
  }
}

void loop() {
  logger.add((long)(millis() / 1000));
  if (!logger.save()) Serial.println(logger.errorText());
  delay(1000);
}
#else
void setup() { Serial.begin(115200); Serial.println(F("This example is for ESP32")); }
void loop() {}
#endif
