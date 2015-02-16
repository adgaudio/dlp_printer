#include "laser.h"

namespace laser {

  void configure() {
    /* Initial settings for Laser pins */
    // TODO
    for (int i=0; i<2; i++) {
      pinMode(LASER_XY_PINS[i], OUTPUT);
    }
    // ensure laser is powered off
    pinMode(LASER_POWER_PIN, OUTPUT);
    digitalWrite(LASER_POWER_PIN, LOW);  // off by default
    // power on laser galvanometers
    pinMode(LASER_GALVO_POWER_PIN, OUTPUT);
    digitalWrite(LASER_GALVO_POWER_PIN, HIGH);
  }

}
