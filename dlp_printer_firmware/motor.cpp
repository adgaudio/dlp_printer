#include "motor.h"

namespace motor {

  void configure() {
    /* Initial settings for Motor pins */
    for (int i=0 ; i<NUM_MOTORS; i++) {
      pinMode(MOTOR_STEP_PINS[i], OUTPUT);
      pinMode(MOTOR_DIR_PINS[i], OUTPUT);
    }
    for (int i=0; i<3; i++) {
      pinMode(MOTOR_MICROSTEP_PINS[i], OUTPUT);
    }
    pinMode(MOTOR_POWER_PIN, OUTPUT);
    digitalWrite(MOTOR_POWER_PIN, HIGH);  // on by default
    set_num_steps_per_turn(MICROSTEPS);
  }


  void set_num_steps_per_turn(int microsteps) {
    /* Configure stepper motor microstepping
     * Please pass exactly 1 byte specifying the number of microsteps
     * per turn: 0, 4, 8, 16, 32
     */
    switch (microsteps) {
      case 0:
        // "Full Step (no microstepping)"
        digitalWrite(MOTOR_MICROSTEP_PINS[0], LOW);
        digitalWrite(MOTOR_MICROSTEP_PINS[1], LOW);
        digitalWrite(MOTOR_MICROSTEP_PINS[2], LOW);
        break;
      case 2:
        // "1/2 Step Microstepping"
        digitalWrite(MOTOR_MICROSTEP_PINS[0], HIGH);
        digitalWrite(MOTOR_MICROSTEP_PINS[1], LOW);
        digitalWrite(MOTOR_MICROSTEP_PINS[2], LOW);
        break;
      case 4:
        // "1/4 Step Microstepping"
        digitalWrite(MOTOR_MICROSTEP_PINS[0], LOW);
        digitalWrite(MOTOR_MICROSTEP_PINS[1], HIGH);
        digitalWrite(MOTOR_MICROSTEP_PINS[2], LOW);
        break;
      case 8:
        // "1/8 Step Microstepping"
        digitalWrite(MOTOR_MICROSTEP_PINS[0], HIGH);
        digitalWrite(MOTOR_MICROSTEP_PINS[1], HIGH);
        digitalWrite(MOTOR_MICROSTEP_PINS[2], LOW);
        break;
      case 16:
        // "1/16 Step Microstepping"
        digitalWrite(MOTOR_MICROSTEP_PINS[0], LOW);
        digitalWrite(MOTOR_MICROSTEP_PINS[1], LOW);
        digitalWrite(MOTOR_MICROSTEP_PINS[2], HIGH);
        break;
      case 32:
        // "1/32 Step Microstepping"
        digitalWrite(MOTOR_MICROSTEP_PINS[0], HIGH);
        digitalWrite(MOTOR_MICROSTEP_PINS[1], HIGH);
        digitalWrite(MOTOR_MICROSTEP_PINS[2], HIGH);
        break;
      default:
        util::fail("Invalid microsteps. Must be one of: 0, 2, 4, 8, 16 or 32");
        break;
    }
  }


  void set_direction(int motor_idx, bool forward) {
    /* Set the direction pin for a particular motor
     *
     * `motor_idx` specifies which motor we're working with
     * `forward` if false, set motor to move backwards
     */
    if (forward) {
      digitalWrite(MOTOR_DIR_PINS[motor_idx], HIGH);
    } else {
      digitalWrite(MOTOR_DIR_PINS[motor_idx], LOW);
    }
  }


  void slide_vat(int motor_idx, long nsteps, int feedrate) {
    /* Slide the vat side to side */
    long delay_bt_steps = nsteps / feedrate;
    while (nsteps-- > 0) {
      digitalWrite(MOTOR_DIR_PINS[motor_idx], HIGH);
      delayMicroseconds(2);
      digitalWrite(MOTOR_DIR_PINS[motor_idx], LOW);
      delayMicroseconds(delay_bt_steps - 2);
    }
  }
}
