// AlashLogger: запись данных датчиков в CSV на SD-карту: заголовок, время, ротация файлов, защита от потери данных.
// Только встроенная SD. MIT License. Alash Electronics, https://github.com/Alash-electronics/AlashLogger
#ifndef ALASH_LOGGER_H
#define ALASH_LOGGER_H

#include <Arduino.h>
#include <SD.h>
#if defined(ESP32)
#include <SPI.h>
#endif

#ifndef ALASH_LOGGER_ROW
#define ALASH_LOGGER_ROW 96        // длина одной строки CSV в байтах (память под буфер)
#endif

class AlashLogger {
 public:
  enum Error : uint8_t { ERR_OK = 0, ERR_NO_CARD, ERR_OPEN, ERR_WRITE, ERR_COLUMNS, ERR_ROW_TOO_LONG, ERR_NAME };
  enum Language : uint8_t { LANG_RU = 0, LANG_KK = 1, LANG_EN = 2 };
  typedef void (*TimeFn)(char *buf, uint8_t size);   // записывает в buf время строкой, например "2026-11-02 12:30:05"

  // csPin: вывод CS модуля SD. baseName: имя файла без расширения (на Arduino AVR до 6 символов): "log" даёт log.csv, log_1.csv...
  bool begin(uint8_t csPin, const char *baseName = "log");
#if defined(ESP32)
  // Модуль SD на других пинах SPI: SPIClass spiSd(HSPI); spiSd.begin(SCK, MISO, MOSI, CS); logger.begin(CS, spiSd, "log");
  bool begin(uint8_t csPin, SPIClass &spi, const char *baseName = "log", uint32_t hz = 4000000UL);
#endif

  // ---- Настройка ---------------------------------------------------------------
  void setColumns(const char *names);          // заголовок после времени: "temp,hum,lux"; число значений в строке будет проверяться
  // Разделитель столбцов. Excel с русскими и казахскими региональными настройками ждёт ";" и десятичную запятую: setSeparator(';', true)
  void setSeparator(char sep, bool decimalComma = false) { _sep = sep; _decComma = decimalComma; }
  void setTimeColumn(const char *name) { _timeName = name; }      // название первого столбца (по умолчанию "time")
  void setTimeProvider(TimeFn fn) { _timeFn = fn; }               // своё время (RTC, NTP); по умолчанию секунды с запуска
  void setMaxFileSize(uint32_t bytes) { _maxSize = bytes; }       // при превышении начнётся новый файл (0 = без ограничения)
  void setFlushEvery(uint16_t ms) { _flushMs = ms; }              // как часто сбрасывать на карту (по умолчанию 5000 мс)

  // ---- Запись --------------------------------------------------------------------
  void add(float v, uint8_t decimals = 2);     // NAN записывается пустой ячейкой
  void add(long v);
  void add(int v) { add((long)v); }
  void add(unsigned long v);
  void add(unsigned int v) { add((unsigned long)v); }
  void add(const char *text);                  // кавычки и запятые экранируются
  bool save();                                 // записать строку: время + добавленные значения
  void flush();                                // сбросить на карту сейчас (перед выключением питания)
  void close();

  // ---- Состояние и диагностика -------------------------------------------------------
  uint32_t rows() const { return _rows; }
  uint32_t fileSize() const { return _size; }
  const char *fileName() const { return _name; }
  uint8_t fileNumber() const { return _num; }
  Error error() const { return _err; }
  const __FlashStringHelper *errorText();
  void setLanguage(Language l) { _lang = l; }
  void diagnose(Print &out);

 private:
  uint8_t _cs = 0, _num = 0, _cells = 0, _cols = 0;
  char _base[10], _name[16];
  char _row[ALASH_LOGGER_ROW];
  uint8_t _len = 0;
  bool _overflow = false, _open = false, _started = false, _dirty = false;
  char _sep = ',';
  bool _decComma = false;
#if defined(ESP32)
  SPIClass *_spi = nullptr;
  uint32_t _hz = 4000000UL;
#endif
  const char *_timeName = "time";
  char _header[48] = "";
  TimeFn _timeFn = nullptr;
  uint32_t _maxSize = 0, _size = 0, _rows = 0, _lastFlush = 0, _lastRetry = 0;
  uint16_t _flushMs = 5000;
  File _file;
  Error _err = ERR_OK;
  Language _lang = LANG_RU;

  void putCell(const char *s, bool quote);
  void startCell();
  bool sdBegin();
  bool openFile();
  bool ensureHeader();
  void makeName();
  bool rotate();
};

#endif  // ALASH_LOGGER_H
