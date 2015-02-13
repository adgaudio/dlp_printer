#ifndef __EEPROM_H_INCLUDED__
#define __EEPROM_H_INCLUDED__

#include "avr/eeprom.h"


namespace eeprom {
  void write(uint8_t start_idx, const uint8_t* data_arr, uint8_t size);
  void read(uint8_t start_idx, uint8_t* data_arr, uint8_t size);
}

#endif  // __EEPROM_H_INCLUDED__
