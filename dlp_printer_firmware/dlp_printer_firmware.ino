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
  /*
  Please encode ints in Big Endian order.
  All further messages must contain the following prefix
    (1 bit: update configuration 1=yes|0=no)  // TODO
    (1 bit: write configuration to EEPROM 1=yes|0=no)  // TODO
    (1 bit: empty)
    (1 bit: motor power on=1|off=0)  // 1<<4
    (1 bit: laser galvos power on=1|off=0)  // 1<<3
    (1 bit: laser power on=1|off=0)  // 1<<2
    (1 bit: move motors? 1=yes|0=no)  // 1<<1
    (1 bit: move laser galvos? 1=yes|0=no)  // 1

    (int_32: num microseconds to apply this action for)

  If move motors == yes or move lasers == yes, message must proceed with:
    (4 bits: empty)
    (1 bit: laser_galvo_y dir 1=forward|0=backward)
    (1 bit: laser_galvo_x dir 1=forward|0=backward)
    (1 bit: motor_2 dir 1=forward|0=backward)
    (1 bit: motor_1 dir 1=forward|0=backward)
  If move motors == yes, message must proceed with:
    (int_32: num steps motor 2)
    (int_32: num steps motor 1)
  If move laser galvos, the message must proceed with:
    (int_12: y position)
    (int_12: x position)
  Happy printing!
  */
}


void loop() {
  wdt_reset (); // reset watchdog counter

  // read data from serial
  if (Serial.available()) {
    byte instructions = util::serial_read_byte();
    unsigned long microsecs = util::serial_read_long();

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
    // TODO: figure out when and how to turn this off safely
    /* if (instructions & 4) {  // power laser BEAM on|off */
      /* digitalWrite(LASER_POWER_PIN, HIGH); */
    /* } else { */
      /* digitalWrite(LASER_POWER_PIN, LOW); */
    /* } */
    unsigned int directions;
    unsigned long step_pins[main::NUM_STEP_PINS];
    for (int i=0; i < main::NUM_STEP_PINS; i++) {
      step_pins[i] = 0;
    }

    if (instructions & 3) {  // receive directions
      // move motors or galvos forward|backward?
      directions = util::serial_read_byte();
    }
    // get num steps to move motors|laser_galvos
    int i;  // pin index for motors and lasers
    if (instructions & 2) {
      // how many steps to move motors?
      for (i=0; i<motor::NUM_MOTORS; i++) {
        step_pins[i] = util::serial_read_long();
      }
    }
    if (instructions & 1) {
      // how many steps to move laser galvos?
      // assume exactly 2 laser galvos exist (x and y)
      byte a = util::serial_read_byte();
      byte b = util::serial_read_byte();
      byte c = util::serial_read_byte();
      i = motor::NUM_MOTORS;
      step_pins[i] = (long) (a << 4) | (b >> 4);  // laser X
      step_pins[++i] = (long) ((b & 0x0F) << 8) | c;  // laser y
    }
    delete &i;
    // TODO: remove debug:
    Serial.println(((String) "stepping for ") + microsecs + " microsecs...");
    step(step_pins, microsecs, directions);
  }
}

void step(unsigned long steps_per_pin[], unsigned long microsecs,
          unsigned int directions) {
  /*
     Move motors and laser galvos simultaneously...

     For each motor pin, pulse a high-low sequence once per step over the
     course of X `microsecs`. For each laser galvo, call the laser::step(...)
     function.  Pulses happen as close to concurrently as
     possible.  Accurate timing is not guaranteed due to interrupts and time
     spent running computations.  You can compensate for this at the
     software level.

     `steps_per_pin` determine how many steps each motor or laser galvo should
       move.  Each element of the array corresponds to motor pins to pulse.
       Pass motor pins first and then laser pins second.
     `microsecs` is a long (4 byte int) specifying how much time to move motors
   */
  unsigned long total_num_steps = util::lcm(steps_per_pin, main::NUM_STEP_PINS);
  unsigned long delay_between_steps = microsecs / total_num_steps - 2;

  if (delay_between_steps >= WDT_DELAY) {
    util::fail("step(): Too much delay between motor pulses. Try increasing the"
        " steps_per_pin values or reducing the total travel time");
  } else if (delay_between_steps < 2) {
    util::fail("step(): Not enough delay between motor pulses.  Try decreasing the"
        " steps_per_pin values or increasing the total travel time");
  }
  // for each pin, a pulse comes every cnt steps
  // initialize a count for each motor
  unsigned int counter_max[main::NUM_STEP_PINS];
  unsigned int counters[main::NUM_STEP_PINS];
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
        laser::step(
            laser_idx,
            (directions >> motor::NUM_MOTORS) & (1<<laser_idx));
      }
    }
    delayMicroseconds(delay_between_steps);
    wdt_reset(); // reset watchdog counter
  }
}
