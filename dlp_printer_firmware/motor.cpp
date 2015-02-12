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
    set_num_steps_per_turn();
  }


  void set_num_steps_per_turn() {
    /* Configure stepper motor microstepping */
    Serial.println("Please pass exactly 1 byte specifying the number of microsteps per turn: 0, 4, 8, 16, 32");
    int byte1 = (int) util::serial_read_byte();
    switch (byte1) {
      case 0:
        Serial.println("Full Step (no microstepping)");
        digitalWrite(MOTOR_MICROSTEP_PINS[0], LOW);
        digitalWrite(MOTOR_MICROSTEP_PINS[1], LOW);
        digitalWrite(MOTOR_MICROSTEP_PINS[2], LOW);
        break;
      case 2:
        Serial.println("1/2 Step Microstepping");
        digitalWrite(MOTOR_MICROSTEP_PINS[0], HIGH);
        digitalWrite(MOTOR_MICROSTEP_PINS[1], LOW);
        digitalWrite(MOTOR_MICROSTEP_PINS[2], LOW);
        break;
      case 4:
        Serial.println("1/4 Step Microstepping");
        digitalWrite(MOTOR_MICROSTEP_PINS[0], LOW);
        digitalWrite(MOTOR_MICROSTEP_PINS[1], HIGH);
        digitalWrite(MOTOR_MICROSTEP_PINS[2], LOW);
        break;
      case 8:
        Serial.println("1/8 Step Microstepping");
        digitalWrite(MOTOR_MICROSTEP_PINS[0], HIGH);
        digitalWrite(MOTOR_MICROSTEP_PINS[1], HIGH);
        digitalWrite(MOTOR_MICROSTEP_PINS[2], LOW);
        break;
      case 16:
        Serial.println("1/16 Step Microstepping");
        digitalWrite(MOTOR_MICROSTEP_PINS[0], LOW);
        digitalWrite(MOTOR_MICROSTEP_PINS[1], LOW);
        digitalWrite(MOTOR_MICROSTEP_PINS[2], HIGH);
        break;
      case 32:
        Serial.println("1/32 Step Microstepping");
        digitalWrite(MOTOR_MICROSTEP_PINS[0], HIGH);
        digitalWrite(MOTOR_MICROSTEP_PINS[1], HIGH);
        digitalWrite(MOTOR_MICROSTEP_PINS[2], HIGH);
        break;
      default:
        util::fail(String ("You passed an invalid byte: ") + (int) byte1 +
            ". You must supply a valid number of stepper motor microsteps: " +
            " 0, 2, 4, 8, 16 or 32");
        break;
    }
  }


  void set_direction(int directions) {
    /* Set the direction pin for each motor according to a bit sequence
       Big Endian Order: the first bit corresponds to the first motor

       `directions` is a 1-byte bitmap for recognized motors.
       */
    for (int jth_bit=0; jth_bit<8; jth_bit++) {
      if (directions & (1<<(7-jth_bit))) {
        digitalWrite(MOTOR_DIR_PINS[jth_bit], HIGH);
      } else {
        digitalWrite(MOTOR_DIR_PINS[jth_bit], LOW);
      }
    }
  }
}
