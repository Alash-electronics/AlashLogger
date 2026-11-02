# AlashLogger

[Русский](#русский) | [Қазақша](#қазақша) | [English](#english)

---

## Русский

Простая библиотека для записи данных датчиков в **CSV на SD-карту**: `setColumns("temp,hum")`, `add(...)` для каждого значения, `save()`. Заголовок, время, новые файлы по размеру и защита от потери данных уже внутри. Файл открывается в Excel и Google Таблицах. Только встроенная `SD`. Сделана для уроков и наборов Alash Electronics.

```cpp
#include <AlashLogger.h>
AlashLogger logger;

void setup() {
  logger.setColumns("temp,hum");
  logger.begin(10, "data");        // CS = 10, файл data.csv
}
void loop() {
  logger.add(23.5f);
  logger.add(41.0f);
  logger.save();                   // строка: время,23.50,41.00
  delay(1000);
}
```

### Что умеет
- **Три шага:** `setColumns`, `add`, `save`. Заголовок пишется один раз сам.
- **Время:** секунды с запуска или своё время из часов (`setTimeProvider`) и своё имя столбца.
- **Файлы не растут бесконечно:** `setMaxFileSize` начинает новый файл (data_1.csv...). После перезапуска запись продолжается в последнем файле, ничего не стирается.
- **Бережно к карте:** сброс на карту по таймеру (`setFlushEvery`), `flush()` перед выключением.
- **Карту вынули и вставили:** ошибка записи, потом запись продолжается сама.
- **Защита от ошибок:** неверное число значений, слишком длинная строка и неверное имя не портят файл.
- **CSV по правилам:** запятые и кавычки в тексте экранируются, `NAN` даёт пустую ячейку, числа с запятой не ломаются.
- **Понятные подсказки** на трёх языках: `errorText()`, `diagnose(Serial)`.
- Работает на **Arduino AVR, ESP32, ESP8266**.
- **Excel без мучений:** `setSeparator(';', true)` для русских и казахских настроек Excel (разделитель `;` и запятая в числах). На ESP32 SD-модуль можно повесить на свои пины SPI: `begin(cs, spi)`.

### Подключение
| Модуль SD | Uno / Nano | ESP32 |
|---|---|---|
| VCC | 5 В | 5 В (или 3.3 В по модулю) |
| GND | GND | GND |
| MOSI | 11 | GPIO23 |
| MISO | 12 | GPIO19 |
| SCK | 13 | GPIO18 |
| CS | 10 | GPIO5 |

Карта FAT16 / FAT32, до 32 ГБ надёжнее всего. Большинство модулей с делителем напряжения рассчитано на 5 В. На AVR имя файла до 6 символов.

### Чем отличается от других библиотек
Мы посмотрели SdFat, ESP32-datalogger-примеры и популярные скетчи с `SD.open(FILE_WRITE)`:
- `FILE_WRITE` на ESP32 стирает файл при каждом запуске, а на AVR дописывает; мы используем правильный режим на каждой плате.
- В большинстве скетчей нет заголовка, ротации и восстановления после вынутой карты.
- `print(float)` с запятой в локалях и `printf("%f")` на AVR; у нас числа пишутся одинаково везде.
- Нет подсказок на казахском.

### Примеры
`Basic`, `Rotate`, `CustomTime`, `Diagnose`, `ExcelRu`, `ESP32_CustomSPI`

### Тесты
SD-карта в памяти (как на ESP32: FILE_WRITE стирает, FILE_APPEND дописывает) проверяет заголовок, форматы чисел, экранирование CSV, число значений, продолжение после перезапуска, ротацию без потери строк, сброс по таймеру, вынутую и вставленную карту, заполненную карту и слишком длинные строки: `extras/test`.

Подробное руководство: [docs/WIKI.md](docs/WIKI.md).

---

## Қазақша

Сенсор деректерін **SD-картаға CSV** түрінде жазуға арналған қарапайым кітапхана: `setColumns("temp,hum")`, әр мәнге `add(...)`, `save()`. Тақырып, уақыт, өлшем бойынша жаңа файлдар мен деректі жоғалтудан қорғау ішінде. Файл Excel және Google Кестелерінде ашылады. Тек кірістірілген `SD`.

```cpp
#include <AlashLogger.h>
AlashLogger logger;

void setup() {
  logger.setColumns("temp,hum");
  logger.begin(10, "data");        // CS = 10, файл data.csv
}
void loop() {
  logger.add(23.5f);
  logger.add(41.0f);
  logger.save();                   // строка: время,23.50,41.00
  delay(1000);
}
```

### Мүмкіндіктері
- **Үш қадам:** `setColumns`, `add`, `save`. Тақырып бір рет өзі жазылады.
- **Уақыт:** іске қосылғаннан бергі секунд не сағаттан өз уақытыңыз (`setTimeProvider`).
- **Файлдар шексіз өспейді:** `setMaxFileSize` жаңа файл бастайды. Қайта қосқаннан кейін соңғы файлда жалғасады.
- **Картаға ұқыпты:** таймер бойынша жазу, өшірер алдында `flush()`.
- **Картаны шығарып салса:** жазу қатесі, кейін өзі жалғасады.
- **Қателерден қорғау:** мән саны қате, тым ұзын жол, қате ат файлды бұзбайды.
- **CSV ережесімен:** үтір мен тырнақша қорғалады, `NAN` бос ұяшық.
- **Түсінікті кеңестер** үш тілде: `errorText()`, `diagnose(Serial)`.
- **Arduino AVR, ESP32, ESP8266**.
- **Excel оңай:** орыс және қазақ Excel баптаулары үшін `setSeparator(';', true)` (бөлгіш `;` және сандағы үтір). ESP32-де SD модулін өз SPI пиндеріне қосуға болады: `begin(cs, spi)`.

### Қосу
| SD модулі | Uno / Nano | ESP32 |
|---|---|---|
| VCC | 5 В | 5 В (или 3.3 В по модулю) |
| GND | GND | GND |
| MOSI | 11 | GPIO23 |
| MISO | 12 | GPIO19 |
| SCK | 13 | GPIO18 |
| CS | 10 | GPIO5 |

Карта FAT16 / FAT32, 32 ГБ-қа дейін сенімдірек. AVR-да файл аты 6 таңбаға дейін.

### Басқа кітапханалардан айырмашылығы
SdFat және `SD.open(FILE_WRITE)` скетчтерін қарадық:
- ESP32-де `FILE_WRITE` файлды өшіреді, AVR-да толықтырады; бізде әр тақтада дұрыс режим.
- Көп скетчте тақырып, ротация, қалпына келу жоқ.
- AVR-да `printf("%f")` жұмыс істемейді; бізде сандар барлық жерде бірдей.
- Қазақша кеңестер жоқ.

### Мысалдар
`Basic`, `Rotate`, `CustomTime`, `Diagnose`, `ExcelRu`, `ESP32_CustomSPI`

### Тесттер
Жадтағы SD-карта тақырыпты, сан пішімдерін, CSV қорғауын, қайта қосудан кейінгі жалғасуды, ротацияны, таймер бойынша жазуды, картаны шығарып салуды тексереді: `extras/test`.

Толық нұсқаулық: [docs/WIKI.md](docs/WIKI.md).

---

## English

A simple library for writing sensor data as **CSV on an SD card**: `setColumns("temp,hum")`, `add(...)` for each value, `save()`. The header, time, new files by size and protection from data loss are built in. The file opens in Excel and Google Sheets. Only the built-in `SD`. Made for classes and Alash Electronics kits.

```cpp
#include <AlashLogger.h>
AlashLogger logger;

void setup() {
  logger.setColumns("temp,hum");
  logger.begin(10, "data");        // CS = 10, файл data.csv
}
void loop() {
  logger.add(23.5f);
  logger.add(41.0f);
  logger.save();                   // строка: время,23.50,41.00
  delay(1000);
}
```

### Features
- **Three steps:** `setColumns`, `add`, `save`. The header is written once by itself.
- **Time:** seconds since start or your own time from a clock (`setTimeProvider`) and your own column name.
- **Files do not grow forever:** `setMaxFileSize` starts a new file (data_1.csv...). After a restart it continues in the last file, nothing is erased.
- **Gentle on the card:** flush on a timer (`setFlushEvery`), `flush()` before power off.
- **Card pulled out and back:** a write error, then logging resumes by itself.
- **Error guards:** a wrong number of values, a too long row or a bad name do not damage the file.
- **Proper CSV:** commas and quotes in text are escaped, `NAN` gives an empty cell, floats never use a comma.
- **Clear tips** in three languages: `errorText()`, `diagnose(Serial)`.
- Works on **Arduino AVR, ESP32, ESP8266**.
- **Easy Excel:** `setSeparator(';', true)` for Russian and Kazakh Excel settings (a `;` separator and a comma in numbers). On ESP32 the SD module can sit on its own SPI pins: `begin(cs, spi)`.

### Wiring
| SD module | Uno / Nano | ESP32 |
|---|---|---|
| VCC | 5 В | 5 В (или 3.3 В по модулю) |
| GND | GND | GND |
| MOSI | 11 | GPIO23 |
| MISO | 12 | GPIO19 |
| SCK | 13 | GPIO18 |
| CS | 10 | GPIO5 |

FAT16 / FAT32 card, up to 32 GB works most reliably. Most modules with a level shifter need 5 V. On AVR the file name is up to 6 characters.

### How it differs from other libraries
We looked at SdFat and the popular sketches with `SD.open(FILE_WRITE)`:
- `FILE_WRITE` erases the file on every start on ESP32 but appends on AVR; we use the right mode on each board.
- Most sketches have no header, rotation or recovery after a pulled card.
- `printf("%f")` does not work on AVR; our numbers are written the same everywhere.
- No Kazakh tips.

### Examples
`Basic`, `Rotate`, `CustomTime`, `Diagnose`, `ExcelRu`, `ESP32_CustomSPI`

### Tests
An in-memory SD card (like on ESP32: FILE_WRITE erases, FILE_APPEND appends) checks the header, number formats, CSV escaping, value count, continuing after a restart, rotation without lost rows, timed flush, a pulled and re-inserted card, a full card and too long rows: `extras/test`.

Full guide: [docs/WIKI.md](docs/WIKI.md).

---

MIT License, Alash Electronics.
