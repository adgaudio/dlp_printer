#ifndef __CONFIGURATION_H_INCLUDED__
#define __CONFIGURATION_H_INCLUDED__

namespace main {
  extern const int BAUDRATE;
}

namespace motor {
  extern const int MOTOR_STEP_PINS[];
  extern const int MOTOR_DIR_PINS[];
  extern const int MOTOR_MICROSTEP_PINS[];
  extern const int NUM_MOTORS; //sizeof(MOTOR_STEP_PINS) / sizeof(MOTOR_STEP_PINS[0]);
  extern const int MOTOR_POWER_PIN;
}

namespace laser {
  extern const int NUM_LASER_MOTORS;
  extern const int LASER_POWER_PIN;
  extern const int LASER_XY_PINS[]; // analog pins
}

#endif  // __CONFIGURATION_H_INCLUDED__
