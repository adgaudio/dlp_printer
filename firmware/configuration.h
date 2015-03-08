#ifndef __CONFIGURATION_H_INCLUDED__
#define __CONFIGURATION_H_INCLUDED__
#include <inttypes.h>

namespace motor {
  extern const int MOTOR_STEP_PINS[];
  extern const int MOTOR_DIR_PINS[];
  extern const int MOTOR_MICROSTEP_PINS[];
  extern const int NUM_MOTORS; //sizeof(MOTOR_STEP_PINS) / sizeof(MOTOR_STEP_PINS[0]);
  extern const int MOTOR_POWER_PIN;
  extern const int MICROSTEPS;
}

namespace laser {
  extern const int NUM_LASER_MOTORS;
  extern const int LASER_POWER_PIN;
  extern const int LASER_GALVO_POWER_PIN;
  extern const uint8_t DEVICE_ADDRS[];
}

namespace main {
  extern const int BAUDRATE;
  extern const int NUM_STEP_PINS;
}

#endif  // __CONFIGURATION_H_INCLUDED__
