#include "configuration.h"


namespace main {
  extern const int BAUDRATE = 9600;
}

///////////////////////////////////////////////////////////////////////////////
//
// Stepper Motor Config: Define which arduino pins correspond to the
// STEP and DIR pins for each motor driver.
//
// Notes:
// - MOTOR_STEP_PINS and MOTOR_DIR_PINS identify the particular driver you wish to control
//   by index.
// - I use Pololu Stepper Motor Driver A4988
// - FYI: you can't configure more than 8 motors at the moment due to the
//   set_direction(...) implementation

// pwm STEP pins ask the stepper driver to move the motor one step
namespace motor {
  extern const int MOTOR_STEP_PINS[] = {8, 10};

  // DIR pins set direction for each stepper motor driver
  extern const int MOTOR_DIR_PINS[] = {9, 11};

  // pins that define how to use microstepping across all stepper drivers
  extern const int MOTOR_MICROSTEP_PINS[] = {5,6,7};

  extern const int NUM_MOTORS = 2;  //sizeof(MOTOR_STEP_PINS) / sizeof(MOTOR_STEP_PINS[0]);

  // The pin that the arduino should use to toggle on or off the motors
  // digitalWrite HIGH --> on.  digitalWrite LOW --> off
  extern const int MOTOR_POWER_PIN = 4;

  // How much resolution to drive motors with.  Higher means more resolution.
  // Options: 0, 4, 8, 16, 32
  extern const int MICROSTEPS = 32;
}
///////////////////////////////////////////////////////////////////////////////
//
// Laser Config
//

// Whether the laser is on or off
namespace laser {
  extern const int NUM_LASER_MOTORS = 2;
  extern const int LASER_POWER_PIN = 3;
  extern const int LASER_GALVO_POWER_PIN = 2;  // TODO: use analog instead?
  extern const int LASER_XY_PINS[] = {1, 2};  // analog pins
}
