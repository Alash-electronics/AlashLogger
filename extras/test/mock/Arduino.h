// Минимальная имитация Arduino.h для запуска тестов на компьютере (g++).
#pragma once
#include <math.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <string>
#define PROGMEM
#define pgm_read_byte(a) (*(const uint8_t *)(a))
#define pgm_read_word(a) (*(const uint16_t *)(a))
class __FlashStringHelper;
#define F(x) (reinterpret_cast<const __FlashStringHelper *>(x))
#define HEX 16
#define DEC 10
inline uint32_t g_ms = 0;
inline uint32_t millis() { return g_ms; }
class String {
 public:
  String(const char *s = "") : s_(s) {}
  const char *c_str() const { return s_.c_str(); }
  std::string s_;
};
class Print {
 public:
  virtual ~Print() {}
  virtual size_t write(uint8_t) = 0;
  virtual size_t write(const uint8_t *b, size_t n) { for (size_t i = 0; i < n; i++) write(b[i]); return n; }
  size_t write(const char *s) { return write((const uint8_t *)s, strlen(s)); }
  size_t print(const char *s) { return write(s); }
  size_t print(const __FlashStringHelper *s) { return write(reinterpret_cast<const char *>(s)); }
  size_t print(char c) { return write((uint8_t)c); }
  size_t print(const String &s) { return write(s.c_str()); }
  size_t print(int v, int base = DEC) { char b[24]; snprintf(b, sizeof b, base == HEX ? "%X" : "%d", v); return write(b); }
  size_t print(unsigned v, int base = DEC) { char b[24]; snprintf(b, sizeof b, base == HEX ? "%X" : "%u", v); return write(b); }
  size_t print(double v, int d = 2) { char b[32]; snprintf(b, sizeof b, "%.*f", d, v); return write(b); }
  size_t println() { return write("\r\n"); }
  size_t println(const char *s) { return print(s) + println(); }
  size_t println(const __FlashStringHelper *s) { return print(s) + println(); }
  size_t println(int v, int base = DEC) { return print(v, base) + println(); }
};
inline void delay(uint32_t ms) { g_ms += ms; }
