// Имитация SD-карты в памяти. Как на ESP32: FILE_WRITE стирает файл, FILE_APPEND дописывает. Можно "вынуть" карту и "заполнить" её.
#pragma once
#include <map>
#include <memory>
#include <string>
#include "Arduino.h"
#ifdef ESP32
#include "SPI.h"
#endif
#define FILE_READ 0
#define FILE_WRITE 1
#define FILE_APPEND 2
struct SimSd {
  bool card = true; size_t capacity = 1u << 30; bool readOnly = false;
  std::map<std::string, std::string> files;
  int begins = 0, flushes = 0, opens = 0;
  const void *lastSpi = nullptr; uint32_t lastHz = 0; uint8_t lastCs = 0;
  size_t used() const { size_t n = 0; for (auto &f : files) n += f.second.size(); return n; }
};
inline SimSd g_sd;
class File : public Print {
 public:
  File() {}
  File(const std::string &n) : _n(n), _ok(true), _gen(g_sd.begins) {}
  explicit operator bool() const { return _ok; }
  size_t write(uint8_t c) override {
    if (!_ok || !g_sd.card || g_sd.readOnly || _gen != g_sd.begins) return 0;
    if (g_sd.used() + 1 > g_sd.capacity) return 0;
    g_sd.files[_n].push_back((char)c);
    return 1;
  }
  size_t write(const uint8_t *b, size_t n) override { size_t w = 0; for (size_t i = 0; i < n; i++) w += write(b[i]); return w; }
  size_t size() { return _ok && g_sd.files.count(_n) ? g_sd.files[_n].size() : 0; }
  void flush() { g_sd.flushes++; }
  void close() { _ok = false; }
 private:
  std::string _n; bool _ok = false; int _gen = 0;
};
class SDClass {
 public:
  bool begin(uint8_t cs = 10) { g_sd.lastCs = cs; g_sd.lastSpi = nullptr; if (!g_sd.card) return false; g_sd.begins++; return true; }
#ifdef ESP32
  bool begin(uint8_t cs, SPIClass &spi, uint32_t hz = 4000000) { g_sd.lastCs = cs; g_sd.lastSpi = &spi; g_sd.lastHz = hz; if (!g_sd.card) return false; g_sd.begins++; return true; }
#endif
  bool exists(const char *n) { return g_sd.files.count(n) > 0; }
  bool remove(const char *n) { return g_sd.files.erase(n) > 0; }
  File open(const char *n, int mode = FILE_READ) {
    if (!g_sd.card || (mode != FILE_READ && g_sd.readOnly)) return File();
    g_sd.opens++;
    if (mode == FILE_WRITE) g_sd.files[n] = "";
    else if (mode == FILE_APPEND) g_sd.files[n];
    else if (!g_sd.files.count(n)) return File();
    return File(n);
  }
};
inline SDClass SD;
