#include "laser.h"

namespace laser {

  void configure() {
    /* Initial settings for Laser pins */
    // TODO
    for (int i=0; i<2; i++) {
      pinMode(LASER_XY_PINS[i], OUTPUT);
    }
    pinMode(LASER_POWER_PIN, OUTPUT);
    digitalWrite(LASER_POWER_PIN, LOW);  // off by default
  }
}
