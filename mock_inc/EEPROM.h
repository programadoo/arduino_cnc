#ifndef MOCK_EEPROM_H
#define MOCK_EEPROM_H

#include <stdint.h>

struct EEPROMClass {
  uint8_t read(int idx) { return 0xFF; }
  void write(int idx, uint8_t val) {}
  void update(int idx, uint8_t val) {}
};

static EEPROMClass EEPROM;

#endif // MOCK_EEPROM_H
