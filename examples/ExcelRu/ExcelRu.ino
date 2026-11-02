// [ru] AlashLogger для Excel с русскими и казахскими региональными настройками: разделитель ";" и десятичная запятая.
//      Файл открывается двойным щелчком сразу столбцами, числа 23,5 считаются числами, а не текстом.
// [kk] AlashLogger орыс және қазақ аймақтық баптаулары бар Excel үшін: ";" бөлгіші және ондық үтір.
//      Файл екі рет шерткенде бірден бағандармен ашылады, 23,5 сандары мәтін емес, сан болып саналады.
// [en] AlashLogger for Excel with Russian or Kazakh regional settings: ";" separator and a decimal comma.
//      The file opens with a double click already split into columns, and 23,5 counts as a number, not text.
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
  logger.setSeparator(';', true);              // разделитель ";" и запятая в числах
  logger.setColumns("temp,hum");
  if (!logger.begin(CS_PIN, "excel")) {
    Serial.println(logger.errorText());
    while (true) delay(1000);
  }
}

void loop() {
  logger.add(20.0f + (millis() / 1000 % 10) * 0.5f, 1);
  logger.add(45.5f, 1);
  if (!logger.save()) Serial.println(logger.errorText());
  delay(1000);
}
