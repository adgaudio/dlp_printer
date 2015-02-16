#ifndef __UTIL_H_INCLUDED__
#define __UTIL_H_INCLUDED__

#include "Arduino.h"
#include "avr/wdt.h"  // watchdog resets avr chip if it hangs

namespace util {
  extern void fail(String msg);
  extern unsigned int gcd(unsigned long a, unsigned long b);
  extern unsigned long lcm(
      unsigned long ints[], int num_ints, bool ignore_zeros=true);
  extern byte serial_read_byte();
  extern unsigned long serial_read_long();
}
#endif  // __UTIL_H_INCLUDED__
