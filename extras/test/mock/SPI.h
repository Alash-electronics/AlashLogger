// Имитация SPI для сборки с -DESP32: объект SPIClass нужен только как адрес, по которому видно, какая шина передана в SD.begin.
#pragma once
#include <stdint.h>
class SPIClass { public: SPIClass(int = 0) {} };
inline SPIClass SPI;
