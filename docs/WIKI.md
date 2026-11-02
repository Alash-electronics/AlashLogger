# AlashLogger: руководство / нұсқаулық / guide

[Русский](#русский) | [Қазақша](#қазақша) | [English](#english)

---

## Русский

Простая библиотека для записи данных датчиков в **CSV на SD-карту**: `setColumns("temp,hum")`, `add(...)` для каждого значения, `save()`. Заголовок, время, новые файлы по размеру и защита от потери данных уже внутри. Файл открывается в Excel и Google Таблицах. Только встроенная `SD`. Сделана для уроков и наборов Alash Electronics.

### Установка
Arduino IDE: Скетч, Подключить библиотеку, Управлять библиотеками, найти **AlashLogger** и нажать Установить. Или скачайте ZIP с GitHub: Скетч, Подключить библиотеку, Добавить .ZIP библиотеку.

### Платы
Arduino Uno / Nano / Mega (AVR), ESP32, ESP8266 и другие, где работает Arduino-ядро.

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

### Быстрый старт
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

### Справочник по методам
| Метод | Что делает |
|---|---|
| `begin(cs, name)` | Открыть файл (name без расширения) |
| `setColumns("a,b")` | Заголовок и проверка числа значений |
| `add(value)` | Добавить значение в строку |
| `save()` | Записать строку |
| `setTimeProvider(fn)` | Своё время (RTC) |
| `setMaxFileSize(bytes)` | Новый файл при превышении |
| `setFlushEvery(ms)` | Как часто сбрасывать на карту |
| `flush() / close()` | Сбросить / закрыть |
| `diagnose(Serial)` | Самопроверка |

### Если что-то не работает
**SD-карта не найдена**

Проверьте MOSI, MISO, SCK, CS, питание модуля и формат карты (FAT16 / FAT32). Запустите Diagnose.

**Файл пустой или не обновляется**

Данные сбрасываются на карту раз в 5 секунд. Перед выключением вызывайте `flush()`, а для проверки `setFlushEvery(0)`.

**В Excel всё в одной колонке**

Excel с русской локалью ждёт точку с запятой. Откройте через Данные, Из текста, разделитель запятая, или используйте Google Таблицы.

### Самопроверка
В библиотеке есть `diagnose(Serial)`: она печатает, что видит плата, и подсказывает, что проверить. Язык подсказок: `setLanguage(LANG_RU / LANG_KK / LANG_EN)`.

### Примеры
`Basic`, `Rotate`, `CustomTime`, `Diagnose`

---

## Қазақша

Сенсор деректерін **SD-картаға CSV** түрінде жазуға арналған қарапайым кітапхана: `setColumns("temp,hum")`, әр мәнге `add(...)`, `save()`. Тақырып, уақыт, өлшем бойынша жаңа файлдар мен деректі жоғалтудан қорғау ішінде. Файл Excel және Google Кестелерінде ашылады. Тек кірістірілген `SD`.

### Орнату
Arduino IDE: Sketch, Include Library, Manage Libraries, **AlashLogger** табып, Install басыңыз. Немесе GitHub-тан ZIP жүктеп, Sketch, Include Library, Add .ZIP Library.

### Тақталар
Arduino Uno / Nano / Mega (AVR), ESP32, ESP8266 және Arduino ядросы бар басқалар.

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

### Жылдам бастау
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

### Әдістер анықтамасы
| Әдіс | Не істейді |
|---|---|
| `begin(cs, name)` | Файлды ашу |
| `setColumns("a,b")` | Тақырып |
| `add(value)` | Мән қосу |
| `save()` | Жолды жазу |
| `setTimeProvider(fn)` | Өз уақытыңыз |
| `setMaxFileSize(bytes)` | Асқанда жаңа файл |
| `setFlushEvery(ms)` | Қаншалықты жиі жазу |
| `flush() / close()` | Жазу / жабу |
| `diagnose(Serial)` | Өзін тексеру |

### Бірдеңе жұмыс істемесе
**SD-карта табылмады**

MOSI, MISO, SCK, CS, қуат және пішімді тексеріңіз.

**Файл бос**

Деректер 5 секунд сайын жазылады. Өшірер алдында `flush()` шақырыңыз.

**Excel-де бәрі бір бағанда**

Деректер, Мәтіннен, үтір бөлгіші арқылы ашыңыз.

### Өзін-өзі тексеру
Кітапханада `diagnose(Serial)` бар: тақта не көргенін басып шығарады және нені тексеру керегін айтады. Кеңес тілі: `setLanguage(LANG_RU / LANG_KK / LANG_EN)`.

### Мысалдар
`Basic`, `Rotate`, `CustomTime`, `Diagnose`

---

## English

A simple library for writing sensor data as **CSV on an SD card**: `setColumns("temp,hum")`, `add(...)` for each value, `save()`. The header, time, new files by size and protection from data loss are built in. The file opens in Excel and Google Sheets. Only the built-in `SD`. Made for classes and Alash Electronics kits.

### Installation
Arduino IDE: Sketch, Include Library, Manage Libraries, search **AlashLogger** and click Install. Or download the ZIP from GitHub: Sketch, Include Library, Add .ZIP Library.

### Boards
Arduino Uno / Nano / Mega (AVR), ESP32, ESP8266 and others with an Arduino core.

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

### Quick start
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

### Method reference
| Method | What it does |
|---|---|
| `begin(cs, name)` | Open the file (name without extension) |
| `setColumns("a,b")` | Header and value count check |
| `add(value)` | Add a value to the row |
| `save()` | Write the row |
| `setTimeProvider(fn)` | Your own time (RTC) |
| `setMaxFileSize(bytes)` | New file when exceeded |
| `setFlushEvery(ms)` | How often to flush |
| `flush() / close()` | Flush / close |
| `diagnose(Serial)` | Self-check |

### If something does not work
**SD card not found**

Check MOSI, MISO, SCK, CS, module power and the card format (FAT16 / FAT32). Run Diagnose.

**The file is empty or not updating**

Data is flushed every 5 seconds. Call `flush()` before power off, and use `setFlushEvery(0)` for testing.

**Everything is in one column in Excel**

A Russian-locale Excel expects a semicolon. Open it with Data, From Text, comma separator, or use Google Sheets.

### Self-check
The library has `diagnose(Serial)`: it prints what the board sees and tells what to check. Tips language: `setLanguage(LANG_RU / LANG_KK / LANG_EN)`.

### Examples
`Basic`, `Rotate`, `CustomTime`, `Diagnose`

---

MIT License, Alash Electronics. https://github.com/Alash-electronics/AlashLogger
