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
#include "eeprom.h"


// reset arduino after 8 seconds.
const unsigned long WDT_DELAY = 8000000;  // consider this non-configurable


void configure_all() {

  // TODO: maybe make a struct containing all fields relevant to configuration
  // and have this link to eeprom somehow
  motor::configure();
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
  configure_all();
  Serial.flush();
  Serial.println("Hello!");
  Serial.flush();
}


void loop() {
  wdt_reset (); // reset watchdog counter

  // read data from serial
  if (Serial.available()) {
    /* Serial.println("get instructions"); */
    // initialize variables
    byte instructions = util::serial_read_byte();
    unsigned long feedrate;
    unsigned int directions;
    unsigned long step_pins[main::NUM_STEP_PINS];
    for (int i=0; i < main::NUM_STEP_PINS; i++) {
      step_pins[i] = 0;
    }

    if (instructions & 1<<7) {  // will update config
      char update_eeprom = instructions & 1<<6;
      util::fail(
          "Cannot dynamically update config or read/write from EEPROM yet.");
      // TODO:
      // read data
      // update global config somehow
      // store to eeprom if necessary
    }

    if (instructions & 16) {  // power motors on|off
      digitalWrite(motor::MOTOR_POWER_PIN, HIGH);
    } else {
      digitalWrite(motor::MOTOR_POWER_PIN, LOW);
    }
    if (instructions & 8) {  // power laser galvanometers on|off
      digitalWrite(laser::LASER_GALVO_POWER_PIN, HIGH);
    } else {
      digitalWrite(laser::LASER_GALVO_POWER_PIN, LOW);
    }

    if (instructions & 4) {  // power laser BEAM on|off
      digitalWrite(laser::LASER_POWER_PIN, HIGH);
    } else {
      digitalWrite(laser::LASER_POWER_PIN, LOW);
    }

    if (instructions & 3) {  // receive feedrate and directions
      /* Serial.println("get feedrate & directions"); */
      // how fast to move motors|galvos?  in microseconds per step
      feedrate = util::serial_read_long();
      // move motors or galvos forward|backward?
      directions = util::serial_read_byte();
    }
    // get num steps to move motors|laser_galvos
    int i;  // pin index for motors and lasers
    if (instructions & 2) {
      /* Serial.println("get motor steps"); */
      // how many steps to move motors?
      for (i=0; i<motor::NUM_MOTORS; i++) {
        step_pins[i] = util::serial_read_long();
      }
    }
    if (instructions & 1) {
      /* Serial.print("get galvo steps"); */
      // how many steps to move laser galvos?
      // assume exactly 2 laser galvos exist (x and y)
      byte a = util::serial_read_byte();
      byte b = util::serial_read_byte();
      byte c = util::serial_read_byte();
      i = motor::NUM_MOTORS;
      step_pins[i] = (((long) a) << 4) | (b >> 4);  // laser X
      step_pins[++i] = (((long) b & 0x0F) << 8) | c;  // laser y
    }
    // TODO: remove debug:
    /* Serial.println((String) "feedrate: " + feedrate); */
    /* Serial.println((String) "Z: " + step_pins[0]); */
    /* Serial.println((String) "S: " + step_pins[1]); */
    /* Serial.println((String) "X: " + step_pins[2]); */
    /* Serial.println((String) "Y: " + step_pins[3]); */
    step(step_pins, feedrate, directions);
    if (directions & 1<<1) {
      /* Serial.println((String) "slide vat " + step_pins[1] + "steps"); */
      // M100 -- swipe motor 2 side to side
      motor::slide_vat(0, step_pins[0], feedrate);
    }
  }
}

void step(unsigned long steps_per_pin[], unsigned long feedrate,
          unsigned int directions) {
  /*
     Move motors and laser galvos simultaneously...

     For each motor pin, pulse a high-low sequence once per step over the
     course of `feedrate` microseconds per step. For each laser galvo, call the
     laser::step(...) function.  Pulses happen as close to concurrently as
     possible.  Accurate timing is not guaranteed due to interrupts and time
     spent running computations.  You can compensate for this at the software
     level by adjusting the feedrate - a constant.

     `steps_per_pin` determine how many steps each motor or laser galvo should
       move.  Each element of the array corresponds to motor pins to pulse.
       Pass motor pins first and then laser pins second.
     `feedrate` is a long (4 byte int) specifying the speed at which to move
       the motors (in # microseconds per step).
   */
  unsigned long total_num_steps = util::lcm(steps_per_pin, main::NUM_STEP_PINS);
  unsigned long delay_between_steps =  // in microseconds
    feedrate * util::maxarr(steps_per_pin, main::NUM_STEP_PINS)
    / total_num_steps;
  /* Serial.print("delay_bt_steps: "); Serial.println(delay_between_steps); */
  /* Serial.println((String) "lcm: " + total_num_steps); */

  if (delay_between_steps >= WDT_DELAY) {
    util::fail(
        "step(): Too much delay between motor pulses. Try increasing the"
        " steps_per_pin values or reducing the total travel time");
  } else if (delay_between_steps < 2) {
    util::fail(
        "step(): Not enough delay between motor pulses.  Try decreasing the"
        " steps_per_pin values or increasing the total travel time");
  }
  // for each pin, a pulse comes every cnt steps
  // initialize a count for each motor
  unsigned long counter_max[main::NUM_STEP_PINS];
  unsigned long counters[main::NUM_STEP_PINS];
  int j;
  for (j=0; j<main::NUM_STEP_PINS ; j++) {
    counter_max[j] = total_num_steps / steps_per_pin[j];
    counters[j] = 0;
  }

  // set motor directions
  for (j=0; j < motor::NUM_MOTORS; j++) {
    motor::set_direction(j, directions & 1<<j);
  }

  // step motors & laser galvos
  for (long step=0; step<total_num_steps; step++) {
    // step motor pins
    for (j=0; j < motor::NUM_MOTORS ; j++) {
      if (++counters[j] >= counter_max[j]) {
        digitalWrite(motor::MOTOR_STEP_PINS[j], HIGH);
      }
    }
    delayMicroseconds(2);
    for (j=0; j < motor::NUM_MOTORS ; j++) {
      if (counters[j] >= counter_max[j]) {
        counters[j] = 0;
        digitalWrite(motor::MOTOR_STEP_PINS[j], LOW);
      }
    }

    // step laser pins
    for (j=motor::NUM_MOTORS; j < main::NUM_STEP_PINS ; j++) {
      if (++counters[j] >= counter_max[j]) {
        int laser_idx = j - motor::NUM_MOTORS;
        laser::step(laser_idx, (directions >> j) & 0x01);
        counters[j] = 0;
      }
    }
    delayMicroseconds(delay_between_steps);
    wdt_reset(); // reset watchdog counter
  }
}
