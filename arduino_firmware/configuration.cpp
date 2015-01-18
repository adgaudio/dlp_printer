#include "configuration.h"
// Stepper Motor Config: Define which arduino pins correspond to the
// STEP and DIR pins for each motor driver.
// Assume using Pololu Stepper Motor Driver A4988
// FYI: you can't configure more than 8 motors at the moment due to the set_direction(...) implementation
int PIN_MAP[] = {5, 7}; // pwm STEP pins ask the stepper driver to move the motor one step
int PIN_DIR_MAP[] = {4, 6}; // DIR pins set direction for each stepper motor driver
int MICROSTEP_PIN_MAP[] = {11, 12, 13}; // pins that define how to use microstepping across all stepper drivers
const int NUM_MOTORS = 2;//sizeof(PIN_MAP) / sizeof(PIN_MAP[0]);

