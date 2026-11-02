// AlashLogger: своё время в первом столбце. Здесь время работы ч:мин:с; с часами реального времени (AlashRTC, RTClib)
// запишите в buf дату и время, например "2026-11-02 12:30:05".
#include <AlashLogger.h>

#if defined(ESP32)
const uint8_t CS_PIN = 5;
#elif defined(ESP8266)
const uint8_t CS_PIN = 15;
#else
const uint8_t CS_PIN = 10;
#endif

AlashLogger logger;

void myTime(char *buf, uint8_t size) {
  unsigned long s = millis() / 1000;
  snprintf(buf, size, "%02lu:%02lu:%02lu", s / 3600, (s / 60) % 60, s % 60);
}

void setup() {
  Serial.begin(115200);
  logger.setTimeProvider(myTime);
  logger.setTimeColumn("uptime");
  logger.setColumns("value");
  if (!logger.begin(CS_PIN, "up")) {
    Serial.println(logger.errorText());
    while (true) delay(1000);
  }
}

void loop() {
  logger.add(analogRead(A0));
  logger.save();
  delay(1000);
}
