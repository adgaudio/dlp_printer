#ifndef __DAC_H_INCLUDED__
#define __DAC_H_INCLUDED__


#include <Wire.h>


namespace dac {
  extern void set_voltage(uint16_t output, uint8_t device_addr);
  extern void step(unsigned long steps_per_pin[], unsigned long microsecs);
}
#endif  // __DAC_H_INCLUDED__
