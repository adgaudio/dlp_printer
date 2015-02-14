/*
   This firmware runs on an arduino and facilitates communication between a
   computer and a dlp printer(containing stepper motors and a laser scanning
   galvanometer).
 */

#include <Wire.h>  // need this for dac.  this lib is weird
#include "avr/wdt.h"  // watchdog resets avr chip if it hangs
#include "configuration.h"
#include "motor.h"
#include "laser.h"
#include "util.h"
#include "dac.h"
#include "eeprom.h"


// reset arduino after 8 seconds.
const unsigned long WDT_DELAY = 8000000;  // consider this non-configurable


void configure_all() {

  int microsteps = (int) util::serial_read_byte();
  // TODO: maybe make a struct containing all fields relevant to configuration
  // and have this link to eeprom somehow
  motor::configure(microsteps);
  laser::configure();
}


void setup() {
  // watchdog: reset after X seconds if counter not reset
  wdt_enable(WDTO_8S);  // TODO: is WDTO_8S the same as WDT_DELAY?

  // serial stuff
  Serial.begin(main::BAUDRATE);
  while (!Serial) {
    ; // wait for serial port to connect. Needed for Leonardo only
  }
  Serial.println("Hello!");
  configure_all();
  Serial.println("Configured");

  /* Serial.println( */
      /* String ("\nYou now control ") + motor::NUM_MOTORS + " stepper" */
      /* " motors and " + laser::NUM_LASER_MOTORS + "lasers.  Please encode ints" */
      /* " in Big Endian order.\n\n"); */
  /* Serial.println( */
      /* (String ("All further messages must contain the following prefix")) + */
      /* "\n   " */
      /* " (1 bit: update configuration 1=yes|0=no)" + */
      /* " (1 bit: write configuration to EEPROM 1=yes|0=no)" + */
      /* " (1 bit: empty)" + */
      /* " (1 bit: motor power on=1|off=0)" +  // 1<<4 */
      /* " (1 bit: laser galvos power on=1|off=0)" +  // 1<<3 */
      /* " (1 bit: laser power on=1|off=0)" +  // 1<<2 */
      /* " (1 bit: move motors? 1=yes|0=no)" +  // 1<<1 */
      /* " (1 bit: move laser galvos? 1=yes|0=no)" +  // 1 */
      /* " (int_32: num microseconds)" + */
      /* "\nIf move motors bit == yes, the message must proceed with:" + */
      /* "\n    (6 bits: empty) (1 bit: motor_2 direction) (1 bit: motor_1 dir)" + */
      /* " (int_32: num steps motor 1) (int_32: num steps motor 2)" + */
      /* "\nNext, if move laser galvos, the message must proceed with:" + */
      /* "\n    (int_12: x position) (int_12: y position)" + */
      /* "\nHappy printing!"); */
  Serial.flush();
}


void loop() {
  wdt_reset (); // reset watchdog counter

  // read data from serial
  if (Serial.available()) {
    byte instructions = util::serial_read_byte();
    unsigned long microsecs = util::serial_read_long();

    if (instructions & 1<<7) {  // will update config
      char update_eeprom = instructions & 1<<6;
      // TODO:
      // read data
      // update global config somehow
      // store to eeprom if necessary
    }
    // TODO only if motors?
    byte directions;
    unsigned long step_pins[motor::NUM_MOTORS];

    if (instructions & 2) {  // will move motors
      directions = util::serial_read_byte();
      for (int i=0; i<motor::NUM_MOTORS; i++) {
        step_pins[i] = util::serial_read_long();
      }
    }
    if (instructions & 1) {  // will move laser galvos

      byte a = util::serial_read_byte();
      byte b = util::serial_read_byte();
      byte c = util::serial_read_byte();
      uint32_t laser_x = (uint32_t) (a << 4) | (b >> 4);
      uint32_t laser_y = (uint32_t) ((b & 0x0F) << 8) | c;
    }

    //  TODO: handle power on|off states
    // " (1 bit: motor power on=1|off=0)" +  // 1<<4
    // " (1 bit: laser galvos power on=1|off=0)" +  // 1<<3
    // " (1 bit: laser power on=1|off=0)" +  // 1<<2

    // TODO:  how to step motors and move lasers at same time?
    //        should probably step the lasers like the motors
    // do stuff with gathered instructions
    // possible idea: overload step(...) for
    // motor only, laser only, and motor + laser
    Serial.println(((String) "stepping for ") + microsecs + " microsecs...");
    motor::set_direction(directions);
    step(step_pins, microsecs);
  }
}

void step(unsigned long steps_per_pin[], unsigned long microsecs) {
  /*
     Move motors simultaneously...

     For each pin, pulse a high-low sequence once per step over the course of X
     `microsecs`. Pulses happen as close to concurrently as possible.
     Accurate timing is not guaranteed.

     `steps_per_pin` is an array of longs (aka 4-byte ints) that determine how
     many steps each motor should move
     `microsecs` is a long (4 byte int) specifying how much time to move motors
   */
  unsigned long total_num_steps = util::lcm(steps_per_pin, motor::NUM_MOTORS);
  unsigned long delay_between_steps = microsecs / total_num_steps - 2;

  if (delay_between_steps > WDT_DELAY) {
    util::fail("step(): Too much delay between motor pulses. Try increasing the"
        " steps_per_pin values or reducing the total travel time");
  } else if (delay_between_steps < 2) {
    util::fail("step(): Not enough delay between motor pulses.  Try decreasing the"
        " steps_per_pin values or increasing the total travel time");
  }
  // for each pin, a pulse comes every cnt steps
  unsigned int counter_max[motor::NUM_MOTORS];
  unsigned int counters[motor::NUM_MOTORS];
  for (int i=0; i<motor::NUM_MOTORS ; i++) {
    counter_max[i] = total_num_steps / steps_per_pin[i];
    counters[i] = 0;
  }
  for (long step=0; step<=total_num_steps; step++) {
    // pulse the pins on each step
    for (int j=0; j < motor::NUM_MOTORS ; j++) {
      if (++counters[j] >= counter_max[j]) {
        digitalWrite(motor::MOTOR_STEP_PINS[j], HIGH);
      }
    }
    delayMicroseconds(2);
    for (int j=0; j < motor::NUM_MOTORS ; j++) {
      if (counters[j] >= counter_max[j]) {
        counters[j] = 0;
        digitalWrite(motor::MOTOR_STEP_PINS[j], LOW);
      }
    }
    delayMicroseconds(delay_between_steps);
    wdt_reset(); // reset watchdog counter
  }
}
