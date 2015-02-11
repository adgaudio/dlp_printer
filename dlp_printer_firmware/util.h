#ifndef __UTIL_H_INCLUDED__
#define __UTIL_H_INCLUDED__

#include "Arduino.h"

namespace util {
  void fail(String msg);
  unsigned int gcd(unsigned long a, unsigned long b);
  unsigned long lcm(unsigned long ints[], int num_ints);
  byte serial_read_byte();
  unsigned long serial_read_long();
}
#endif  // __UTIL_H_INCLUDED__
