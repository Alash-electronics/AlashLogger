// Тесты AlashLogger без железа (SD-карта в памяти, как на ESP32: FILE_WRITE стирает, FILE_APPEND дописывает):
//   g++ -std=c++17 -Wall -I mock -I ../../src test_logger.cpp ../../src/AlashLogger.cpp && ./a.out
#include <stdio.h>
#include "Arduino.h"
#include "SD.h"
#include "AlashLogger.h"

static int fails = 0;
#define CHECK(cond) do { if (!(cond)) { printf("FAIL line %d: %s\n", __LINE__, #cond); fails++; } } while (0)
static void fresh() { g_sd = SimSd(); g_ms = 5000; }
static std::string &file(const char *n) { return g_sd.files[n]; }
static int lines(const std::string &s) { int n = 0; for (char c : s) n += c == '\n'; return n; }
static void myTime(char *b, uint8_t n) { snprintf(b, n, "2026-11-02 12:30:05"); }

int main() {
  // 1) заголовок, время с запуска, значения
  fresh();
  AlashLogger L;
  CHECK(L.begin(10, "data"));
  L.setColumns("temp,hum");
  CHECK(L.begin(10, "data"));                  // второй begin: заголовок ещё не записан, файл пуст
  L.add(23.456f); L.add(41.0f, 0);
  CHECK(L.save());
  CHECK(file("data.csv") == "time,temp,hum\n5.000,23.46,41\n");
  g_ms = 6250; L.add(-0.04f, 1); L.add(7L); CHECK(L.save());
  CHECK(file("data.csv").find("6.250,0.0,7\n") != std::string::npos);     // -0.04 при 1 знаке = 0.0, без минуса
  CHECK(L.rows() == 1 + 1 || L.rows() == 2);

  // 2) форматирование чисел
  fresh(); AlashLogger F; F.begin(10, "f"); F.setColumns("a,b,c,d,e,f,g,h,i"); F.begin(10, "f");
  F.add(0.5f); F.add(-12.345f, 3); F.add(1.005f, 2); F.add(99.995f, 2); F.add(123456.789f, 1); F.add(NAN); F.add(INFINITY); F.add(-3.0f, 0); F.add(0.0f, 4);
  CHECK(F.save());
  std::string row = file("f.csv").substr(file("f.csv").find('\n') + 1);
  size_t comma = row.find(','); row = row.substr(comma + 1);
  CHECK(row == "0.50,-12.345,1.00,100.00,123456.8,,inf,-3,0.0000\n" || row == "0.50,-12.345,1.01,100.00,123456.8,,inf,-3,0.0000\n");
  printf("float row: %s", row.c_str());

  // 3) целые и строки: кавычки, запятые, переводы строк
  fresh(); AlashLogger S; S.begin(10, "s"); S.setColumns("n,u,t"); S.begin(10, "s");
  S.add(-5); S.add(4000000000UL); S.add("a,b \"q\"\n"); CHECK(S.save());
  CHECK(file("s.csv").find("-5,4000000000,\"a,b \"\"q\"\"\n\"\n") != std::string::npos);
  S.add(1); S.add(2); S.add("простая строка Қаз"); CHECK(S.save());
  CHECK(file("s.csv").find(",простая строка Қаз\n") != std::string::npos);

  // 4) число значений не совпадает: строка не пишется, следующая нормальная
  fresh(); AlashLogger C; C.begin(10, "c"); C.setColumns("a,b"); C.begin(10, "c");
  C.add(1); CHECK(!C.save() && C.error() == AlashLogger::ERR_COLUMNS);
  C.add(1); C.add(2); C.add(3); CHECK(!C.save() && C.error() == AlashLogger::ERR_COLUMNS);
  C.add(1); C.add(2); CHECK(C.save() && C.error() == AlashLogger::ERR_OK);
  CHECK(lines(file("c.csv")) == 2);              // заголовок и одна строка
  // без setColumns число значений любое
  fresh(); AlashLogger N; N.begin(10, "n"); N.add(1); N.add(2); N.add(3); CHECK(N.save()); N.add(1); CHECK(N.save());
  CHECK(file("n.csv").find(",1,2,3\n") != std::string::npos);
  N.save(); CHECK(file("n.csv").substr(file("n.csv").rfind('\n', file("n.csv").size() - 2) + 1).find(',') == std::string::npos);   // пустая строка: только время

  // 5) продолжение после перезапуска: файл не стирается (FILE_APPEND), заголовок не дублируется
  fresh(); { AlashLogger A; A.begin(10, "r"); A.setColumns("x"); A.begin(10, "r"); A.add(1); A.save(); A.add(2); A.save(); A.close(); }
  { AlashLogger B; B.setColumns("x"); B.begin(10, "r"); B.add(3); B.save(); B.close(); }
  CHECK(lines(file("r.csv")) == 4);              // заголовок + 3 строки
  CHECK(file("r.csv").find("time,x") == 0 && file("r.csv").rfind("time,x") == 0);

  // 6) ротация по размеру: новые файлы с заголовком, ничего не теряется
  fresh(); AlashLogger R; R.setColumns("v"); R.begin(10, "rot"); R.setMaxFileSize(100);
  for (int i = 0; i < 40; i++) { g_ms += 1000; R.add(i); CHECK(R.save()); }
  CHECK(g_sd.files.count("rot.csv") && g_sd.files.count("rot_1.csv") && g_sd.files.count("rot_2.csv"));
  size_t totalRows = 0;
  for (auto &f : g_sd.files) {
    CHECK(f.second.size() <= 100 + 20);
    CHECK(f.second.find("time,v\n") == 0);
    totalRows += lines(f.second) - 1;
  }
  CHECK(totalRows == 40);
  printf("файлов: %d, последний %s\n", (int)g_sd.files.size(), R.fileName());
  CHECK(R.fileNumber() >= 2 && (int)g_sd.files.size() == R.fileNumber() + 1);
  // после перезапуска продолжаем в последнем файле
  { AlashLogger R2; R2.setColumns("v"); R2.begin(10, "rot"); CHECK(strcmp(R2.fileName(), R.fileName()) == 0); }
  // размер файла из mock совпадает с учётом библиотеки
  CHECK(R.fileSize() == file(R.fileName()).size());

  // 7) своё время
  fresh(); AlashLogger Tm; Tm.begin(10, "t"); Tm.setTimeProvider(myTime); Tm.setTimeColumn("datetime"); Tm.setColumns("v"); Tm.begin(10, "t");
  Tm.add(1); CHECK(Tm.save());
  CHECK(file("t.csv") == "datetime,v\n2026-11-02 12:30:05,1\n");

  // 8) сброс на карту: по таймеру, не каждую строку
  fresh(); AlashLogger Fl; Fl.begin(10, "fl"); Fl.setFlushEvery(5000);
  g_sd.flushes = 0;
  for (int i = 0; i < 10; i++) { g_ms += 100; Fl.add(i); Fl.save(); }
  CHECK(g_sd.flushes == 0);
  g_ms += 5000; Fl.add(1); Fl.save(); CHECK(g_sd.flushes == 1);
  Fl.add(2); Fl.save(); CHECK(g_sd.flushes == 1);
  Fl.flush(); CHECK(g_sd.flushes == 2);          // есть несброшенная строка
  Fl.flush(); CHECK(g_sd.flushes == 2);          // повторно ничего нового: лишних операций нет
  Fl.setFlushEvery(0); g_sd.flushes = 0; Fl.add(1); Fl.save(); Fl.add(1); Fl.save(); CHECK(g_sd.flushes == 2);

  // 9) нет карты, неверное имя
  fresh(); g_sd.card = false; AlashLogger Nc; CHECK(!Nc.begin(10) && Nc.error() == AlashLogger::ERR_NO_CARD);
  Nc.add(1); CHECK(!Nc.save());
  fresh(); AlashLogger Bn; CHECK(!Bn.begin(10, "") && Bn.error() == AlashLogger::ERR_NAME);
  CHECK(!Bn.begin(10, "a.b") && Bn.error() == AlashLogger::ERR_NAME);
  CHECK(!Bn.begin(10, "a b") && Bn.error() == AlashLogger::ERR_NAME);
  CHECK(!Bn.begin(10, "a/b") && Bn.error() == AlashLogger::ERR_NAME);
  CHECK(!Bn.begin(10, "waytoolongname") && Bn.error() == AlashLogger::ERR_NAME);
  fresh(); g_sd.readOnly = true; AlashLogger Ro; CHECK(!Ro.begin(10) && Ro.error() == AlashLogger::ERR_OPEN);

  // 10) карту вынули во время записи и вставили обратно: ошибка записи, потом восстановление без перезапуска
  fresh(); AlashLogger Rm; Rm.begin(10, "rm"); Rm.add(1); CHECK(Rm.save());
  g_sd.card = false; g_ms += 10;
  Rm.add(2); CHECK(!Rm.save() && Rm.error() == AlashLogger::ERR_WRITE);
  g_ms += 100; Rm.add(3); CHECK(!Rm.save());                  // карты нет: попытки не чаще раза в 2 секунды
  int opensBefore = g_sd.opens;
  g_ms += 100; Rm.add(3); CHECK(!Rm.save()); CHECK(g_sd.opens == opensBefore);
  g_sd.card = true; g_ms += 2500;
  Rm.add(4); CHECK(Rm.save() && Rm.error() == AlashLogger::ERR_OK);
  CHECK(file("rm.csv").find(",1\n") != std::string::npos && file("rm.csv").find(",4\n") != std::string::npos && file("rm.csv").find(",2\n") == std::string::npos);

  // 11) карта заполнилась: строка не сохранена, ошибка, не зависает
  fresh(); AlashLogger Fu; Fu.begin(10, "fu"); Fu.add(1); CHECK(Fu.save());
  g_sd.capacity = g_sd.used() + 5;
  Fu.add(123456); CHECK(!Fu.save() && Fu.error() == AlashLogger::ERR_WRITE);

  // 12) слишком длинная строка не портит файл
  fresh(); AlashLogger Lg; Lg.begin(10, "lg"); Lg.setColumns("a,b"); Lg.begin(10, "lg");
  char big[200]; memset(big, 'x', 199); big[199] = 0;
  Lg.add(big); Lg.add(1); CHECK(!Lg.save() && Lg.error() == AlashLogger::ERR_ROW_TOO_LONG);
  CHECK(file("lg.csv").empty());                  // плохая строка ничего не записала
  Lg.add(1); Lg.add(2); CHECK(Lg.save()); CHECK(lines(file("lg.csv")) == 2);
  char mid[80]; memset(mid, 'y', 79); mid[79] = 0;
  Lg.add(mid); Lg.add(mid); CHECK(!Lg.save());                  // 2 x 80 не помещается в 96
  Lg.add(mid); Lg.add(1); CHECK(Lg.save());

  // 13) тексты ошибок на трёх языках
  const AlashLogger::Language langs[3] = {AlashLogger::LANG_RU, AlashLogger::LANG_KK, AlashLogger::LANG_EN};
  for (int e = 0; e < 3; e++) {
    const char *tx[3];
    for (int l = 0; l < 3; l++) {
      fresh(); AlashLogger Le; Le.setLanguage(langs[l]);
      if (e == 0) { g_sd.card = false; Le.begin(10); }
      if (e == 1) { Le.begin(10, ""); }
      if (e == 2) { Le.begin(10, "e"); Le.setColumns("a,b"); Le.add(1); Le.save(); }
      tx[l] = (const char *)Le.errorText();
    }
    CHECK(strlen(tx[0]) > 10 && strcmp(tx[0], tx[1]) != 0 && strcmp(tx[1], tx[2]) != 0);
  }

  // 14) diagnose
  fresh(); AlashLogger D; D.begin(10, "dg"); D.add(1); D.save();
  struct Out : Print { size_t write(uint8_t c) override { putchar(c); return 1; } } out;
  printf("--- diagnose ---\n"); D.diagnose(out);
  g_sd.readOnly = true; D.diagnose(out);
  fresh(); g_sd.card = false; AlashLogger D2; D2.begin(10); D2.diagnose(out);
  printf("--- end ---\n");
  CHECK(!g_sd.files.count("alashtst.tmp"));

  // 15) разделитель для Excel с русской / казахской локалью: ";" и десятичная запятая; заголовок, кавычки, ячейки с запятой
  fresh(); AlashLogger X; X.setSeparator(';', true); X.setColumns("temp,hum,note"); X.begin(10, "x");
  X.add(23.456f, 1); X.add(-0.04f, 2); X.add("a;b,c"); CHECK(X.save());
  X.add(1.5f, 0); X.add(NAN); X.add("plain"); CHECK(X.save());
  { std::string f = file("x.csv");
    CHECK(f.rfind("time;temp;hum;note\n", 0) == 0);
    CHECK(f.find(";23,5;-0,04;\"a;b,c\"\n") != std::string::npos);
    CHECK(f.find(";2;;plain\n") != std::string::npos);                         // 1.5 без знаков после запятой даёт 2, NAN пустая ячейка
    CHECK(f.find("5,000;") != std::string::npos || f.find("5,") != std::string::npos);   // время с запуска тоже с запятой
    CHECK(f.find("23.5") == std::string::npos && lines(f) == 3); }
  // запятая как десятичный знак при запятой-разделителе: число и время в кавычках, чтобы столбцы не поехали
  fresh(); AlashLogger Y; Y.setSeparator(',', true); Y.setColumns("v"); Y.begin(10, "y");
  Y.add(2.5f, 1); CHECK(Y.save());
  CHECK(file("y.csv").find(",\"2,5\"\n") != std::string::npos && file("y.csv").find("\"5,000\",") != std::string::npos);
  // по умолчанию всё как раньше: запятая и точка
  fresh(); AlashLogger Z; Z.setColumns("v"); Z.begin(10, "z"); Z.add(2.5f, 1); CHECK(Z.save());
  CHECK(file("z.csv").find("time,v\n") == 0 && file("z.csv").find(",2.5\n") != std::string::npos);
  // своё время со своей запятой при разделителе-запятой оборачивается в кавычки
  fresh(); AlashLogger W; W.setTimeProvider([](char *b, uint8_t n) { snprintf(b, n, "12:30, Mon"); }); W.begin(10, "w"); W.add(1); CHECK(W.save());
  CHECK(file("w.csv").find("\"12:30, Mon\",1\n") != std::string::npos);
  // ротация с другим разделителем сохраняет заголовок с ним
  fresh(); AlashLogger R2; R2.setSeparator(';'); R2.setColumns("a,b"); R2.setMaxFileSize(40); R2.begin(10, "r");
  for (int i = 0; i < 6; i++) { R2.add(i); R2.add(i * 2); CHECK(R2.save()); }
  CHECK(R2.fileNumber() >= 1 && file("r_1.csv").rfind("time;a;b\n", 0) == 0);
#ifdef ESP32
  // 16) своя шина SPI: SD.begin получает её и частоту; повторная попытка после вынутой карты идёт по той же шине
  fresh(); SPIClass hspi(2); AlashLogger H; CHECK(H.begin(15, hspi, "h", 8000000UL));
  CHECK(g_sd.lastSpi == &hspi && g_sd.lastHz == 8000000UL && g_sd.lastCs == 15);
  H.add(1); CHECK(H.save());
  g_sd.card = false; g_ms += 3000; H.add(2); H.save();
  g_sd.card = true; g_ms += 3000; g_sd.lastSpi = nullptr; H.add(3); CHECK(H.save() && g_sd.lastSpi == &hspi && g_sd.lastHz == 8000000UL);
  fresh(); AlashLogger H2; CHECK(H2.begin(5, "h2") && g_sd.lastSpi == nullptr && g_sd.lastCs == 5);   // без своей шины: обычный SD.begin
  fresh(); g_sd.card = false; AlashLogger H3; CHECK(!H3.begin(15, hspi, "h3") && H3.error() == AlashLogger::ERR_NO_CARD);
#endif

  if (fails) { printf("FAILED: %d\n", fails); return 1; }
  printf("ALL TESTS PASSED\n");
  return 0;
}
