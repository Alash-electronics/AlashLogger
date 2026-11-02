// MIT License. Alash Electronics.
#include "AlashLogger.h"
#include <string.h>

#define T(ru, kk, en) (_lang == LANG_KK ? F(kk) : _lang == LANG_EN ? F(en) : F(ru))

// На ESP32 FILE_WRITE стирает файл, на AVR добавляет в конец; FILE_APPEND есть только там, где он нужен
#ifdef FILE_APPEND
#define ALASH_OPEN_MODE FILE_APPEND
#else
#define ALASH_OPEN_MODE FILE_WRITE
#endif

bool AlashLogger::sdBegin() {
#if defined(ESP32)
  if (_spi) return SD.begin(_cs, *_spi, _hz);
#endif
  return SD.begin(_cs);
}

#if defined(ESP32)
bool AlashLogger::begin(uint8_t csPin, SPIClass &spi, const char *baseName, uint32_t hz) {
  _spi = &spi;
  _hz = hz;
  return begin(csPin, baseName);
}
#endif

bool AlashLogger::begin(uint8_t csPin, const char *baseName) {
  _cs = csPin;
  _err = ERR_OK;
  _num = 0;
  _rows = 0;
  size_t n = strlen(baseName);
#ifdef __AVR__
  const size_t maxName = 6;
#else
  const size_t maxName = sizeof(_base) - 1;
#endif
  if (n == 0 || n > maxName) { _err = ERR_NAME; return false; }
  for (size_t i = 0; i < n; i++) {
    char c = baseName[i];
    if (c == '/' || c == '.' || c == ' ' || c == '\\') { _err = ERR_NAME; return false; }
  }
  memcpy(_base, baseName, n + 1);
  if (!sdBegin()) { _err = ERR_NO_CARD; return false; }
  _started = true;
  _lastFlush = millis();
  // продолжить в последнем существующем файле серии (после перезапуска не начинаем заново)
  makeName();
  while (SD.exists(_name) && _num < 99) { _num++; makeName(); }
  if (_num > 0) { _num--; makeName(); }
  return openFile();
}

void AlashLogger::makeName() {
  if (_num == 0) snprintf(_name, sizeof _name, "%s.csv", _base);
  else snprintf(_name, sizeof _name, "%s_%u.csv", _base, (unsigned)_num);
}

void AlashLogger::setColumns(const char *names) {
  strncpy(_header, names, sizeof(_header) - 1);
  _header[sizeof(_header) - 1] = 0;
  _cols = 1;
  for (const char *p = names; *p; p++) if (*p == ',') _cols++;
  if (!*names) _cols = 0;
}

bool AlashLogger::openFile() {
  if (_open) { _file.close(); _open = false; }
  _file = SD.open(_name, ALASH_OPEN_MODE);
  if (!_file) { _err = ERR_OPEN; return false; }
  _open = true;
  _size = _file.size();
  return true;
}

// Заголовок пишется перед первой строкой нового файла, поэтому setColumns() можно вызвать до или после begin()
bool AlashLogger::ensureHeader() {
  if (_size != 0 || !_header[0]) return true;
  size_t w = _file.print(_timeName);
  w += _file.print(_sep);
  for (const char *p = _header; *p; p++) w += _file.write((uint8_t)(*p == ',' ? _sep : *p));   // в setColumns() имена всегда через запятую
  w += _file.write((uint8_t)'\n');
  size_t total = strlen(_timeName) + 1 + strlen(_header) + 1;
  if (w < total) { _err = ERR_WRITE; _file.close(); _open = false; return false; }
  _size = total;
  _dirty = true;
  return true;
}

bool AlashLogger::rotate() {
  if (_num >= 99) { _err = ERR_WRITE; return false; }
  _file.close();
  _open = false;
  _num++;
  makeName();
  return openFile();
}

void AlashLogger::startCell() {
  if (_len < sizeof(_row) - 1 && _cells > 0) _row[_len++] = _sep;
  else if (_cells > 0) _overflow = true;
}

void AlashLogger::putCell(const char *s, bool quote) {
  size_t n = strlen(s);
  if (_len + n + 3 + (quote ? n : 0) >= sizeof(_row)) { _overflow = true; _cells++; return; }
  startCell();
  if (quote) {
    _row[_len++] = '"';
    for (size_t i = 0; i < n; i++) { if (s[i] == '"') _row[_len++] = '"'; _row[_len++] = s[i]; }
    _row[_len++] = '"';
  } else {
    memcpy(_row + _len, s, n);
    _len += n;
  }
  _cells++;
}

void AlashLogger::add(const char *text) {
  bool q = false;
  for (const char *p = text; *p; p++) if (*p == ',' || *p == _sep || *p == '"' || *p == '\n' || *p == '\r') q = true;
  putCell(text, q);
}

void AlashLogger::add(long v) { char b[14]; snprintf(b, sizeof b, "%ld", v); putCell(b, false); }
void AlashLogger::add(unsigned long v) { char b[14]; snprintf(b, sizeof b, "%lu", v); putCell(b, false); }

// Вывод float без printf("%f") (на AVR он не работает)
void AlashLogger::add(float v, uint8_t decimals) {
  if (isnan(v)) { putCell("", false); return; }
  char b[24];
  uint8_t i = 0;
  if (isinf(v)) { putCell(v < 0 ? "-inf" : "inf", false); return; }
  if (decimals > 6) decimals = 6;
  bool neg = v < 0;
  if (neg) v = -v;
  if (v > 2000000000.0f) { putCell(neg ? "-ovf" : "ovf", false); return; }
  uint32_t mult = 1;
  for (uint8_t d = 0; d < decimals; d++) mult *= 10;
  double scaled = (double)v * mult + 0.5;
  uint32_t whole = (uint32_t)(scaled / mult);
  uint32_t frac = (uint32_t)(scaled - (double)whole * mult);
  if (frac >= mult) { whole++; frac -= mult; }
  if (neg && (whole || frac)) b[i++] = '-';
  char tmp[11]; uint8_t t = 0;
  do { tmp[t++] = '0' + whole % 10; whole /= 10; } while (whole);
  while (t) b[i++] = tmp[--t];
  if (decimals) {
    b[i++] = _decComma ? ',' : '.';
    for (int8_t d = decimals - 1; d >= 0; d--) { b[i + d] = '0' + frac % 10; frac /= 10; }
    i += decimals;
  }
  b[i] = 0;
  putCell(b, _decComma && decimals && _sep == ',');   // запятая внутри числа при запятой-разделителе: ячейку берём в кавычки
}

bool AlashLogger::save() {
  uint8_t cells = _cells, len = _len;
  bool overflow = _overflow;
  _len = 0; _cells = 0; _overflow = false;   // строка забрана в любом случае (буфер _row не меняется до следующего add)
  if (!_started) { _err = ERR_NO_CARD; return false; }
  if (overflow) { _err = ERR_ROW_TOO_LONG; return false; }
  if (_cols && cells != _cols) { _err = ERR_COLUMNS; return false; }
  if (!_open) {                                  // карта была вынута: раз в 2 секунды пробуем вернуться
    if (_lastRetry && millis() - _lastRetry < 2000) return false;
    _lastRetry = millis() ? millis() : 1;
    if (!sdBegin()) { _err = ERR_NO_CARD; return false; }
    if (!openFile()) return false;
  }
  char stamp[26];
  if (_timeFn) { stamp[0] = 0; _timeFn(stamp, 24); stamp[23] = 0; }
  else { uint32_t ms = millis(); snprintf(stamp, 24, "%lu%c%03u", (unsigned long)(ms / 1000), _decComma ? ',' : '.', (unsigned)(ms % 1000)); }
  if (_sep == ',' && strchr(stamp, ',')) {      // время со своей запятой при запятой-разделителе: в кавычки
    size_t n = strlen(stamp);
    memmove(stamp + 1, stamp, n);
    stamp[0] = '"'; stamp[n + 1] = '"'; stamp[n + 2] = 0;
  }
  size_t total = strlen(stamp) + (len ? 1 : 0) + len + 1;
  if (_maxSize && _size + total > _maxSize && _size > 0 && !rotate()) return false;
  if (!ensureHeader()) return false;
  size_t w = _file.print(stamp);
  if (len) { w += _file.print(_sep); w += _file.write((const uint8_t *)_row, len); }
  w += _file.write((uint8_t)'\n');
  if (w < total) { _err = ERR_WRITE; _file.close(); _open = false; return false; }
  _size += total;
  _rows++;
  _dirty = true;
  _err = ERR_OK;
  if (_flushMs == 0 || millis() - _lastFlush >= _flushMs) flush();
  return true;
}

void AlashLogger::flush() {
  if (_open && _dirty) { _file.flush(); _dirty = false; }
  _lastFlush = millis();
}

void AlashLogger::close() {
  if (_open) { _file.close(); _open = false; }
}

const __FlashStringHelper *AlashLogger::errorText() {
  switch (_err) {
    case ERR_OK: return F("OK");
    case ERR_NO_CARD:
      return T("SD-карта не найдена: проверьте провода (MOSI, MISO, SCK, CS), питание модуля (5 В для большинства) и формат карты FAT16 / FAT32",
               "SD-карта табылмады: сымдарды (MOSI, MISO, SCK, CS), модуль қуатын (көбіне 5 В) және FAT16 / FAT32 пішімін тексеріңіз",
               "SD card not found: check the wires (MOSI, MISO, SCK, CS), module power (5 V for most) and the FAT16 / FAT32 format");
    case ERR_OPEN:
      return T("Не удалось открыть файл: карта защищена от записи или переполнена", "Файлды ашу мүмкін болмады: карта жазудан қорғалған не толған",
               "Could not open the file: the card is write protected or full");
    case ERR_WRITE:
      return T("Ошибка записи: карту вынули или она заполнена. Данные этой строки не сохранены", "Жазу қатесі: карта алынды не толды. Бұл жол сақталмады",
               "Write error: the card was removed or is full. This row was not saved");
    case ERR_COLUMNS:
      return T("Число значений не совпадает с setColumns(): строка не записана", "Мәндер саны setColumns() бойынша емес: жол жазылмады",
               "The number of values does not match setColumns(): the row was not written");
    case ERR_ROW_TOO_LONG:
      return T("Строка слишком длинная (ALASH_LOGGER_ROW байт): строка не записана", "Жол тым ұзын (ALASH_LOGGER_ROW байт): жол жазылмады",
               "The row is too long (ALASH_LOGGER_ROW bytes): the row was not written");
    case ERR_NAME:
      return T("Неверное имя файла: до 6 символов на AVR, без точки, пробела и слеша", "Файл аты қате: AVR-да 6 таңбаға дейін, нүктесіз, бос орынсыз",
               "Wrong file name: up to 6 characters on AVR, no dot, space or slash");
  }
  return F("");
}

void AlashLogger::diagnose(Print &out) {
  out.println(T("AlashLogger: диагностика", "AlashLogger: диагностика", "AlashLogger: diagnostics"));
  if (!_started) { out.println(errorText()); return; }
  out.print(T("Файл: ", "Файл: ", "File: "));
  out.println(_name);
  out.print(T("Размер, байт: ", "Өлшемі, байт: ", "Size, bytes: "));
  out.println((int)_size);
  out.print(T("Строк записано: ", "Жазылған жол: ", "Rows written: "));
  out.println((int)_rows);
  AlashLogger::Error before = _err;
  File t = SD.open("alashtst.tmp", ALASH_OPEN_MODE);
  if (t) {
    size_t w = t.print(F("test"));
    t.close();
    SD.remove("alashtst.tmp");
    out.println(w == 4 ? T("Запись на карту работает.", "Картаға жазу жұмыс істейді.", "Writing to the card works.")
                        : T("Запись неполная: карта заполнена или повреждена.", "Жазу толық емес: карта толған не бүлінген.", "Incomplete write: the card is full or damaged."));
  } else {
    out.println(T("Файл создать нельзя: карта защищена от записи, переполнена или не FAT.", "Файл жасау мүмкін емес: карта қорғалған, толған не FAT емес.", "Cannot create a file: the card is write protected, full or not FAT."));
  }
  _err = before;
  if (_err != ERR_OK) out.println(errorText());
}
