#include "util.h"


namespace util {


  byte serial_read_byte() {
    /* read a byte from Arduino Serial */
    while (!Serial.available()) {}
    return Serial.read();
  }

  unsigned long serial_read_long() {
    /* read a 4 byte unsigned int (ie an unsigned long) from Serial */
    unsigned long a = 0;
    unsigned long b;
    for (int i=3 ; i>=0 ; i--) {
      b = serial_read_byte();
      a |= (b << (i*8));
    }
    return a;
  }

  void fail(String msg="<unknown error>") {
    /* force hang & reset */
    Serial.println("Fail: " + msg);
    wdt_enable(WDTO_15MS);
    while (1) { }
  }


  unsigned int gcd(unsigned long a, unsigned long b) {
    /* Find the Greatest Common Divisor of two ints */
    unsigned long t;
    while (b != 0) {
      t = a % b;
      a = b;
      b = t;
    }
    return a;
  }


  unsigned long lcm(
      unsigned long ints[], int num_ints, bool ignore_zeros) {
    /* Find the Lowest Common Multiple of an array of ints.
     * Ignore zeros in values by default.
     *
     * lcm = a*b / gcd(a, b)
     * lcm_of_many_ints = reduce(lcm, ints)
     */
    unsigned long lcm_val = 1;
    for (int i=0 ; i < num_ints ; i++) {
      if (ints[i] != 0) {
        lcm_val = lcm_val * (ints[i] / gcd(lcm_val, ints[i]));
      } else if (!ignore_zeros) {
        return 0;
      }
    }
    return lcm_val;
  }
}
