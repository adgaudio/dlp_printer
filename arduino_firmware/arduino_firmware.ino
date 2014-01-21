#include "avr/wdt.h"  // watchdog resets avr chip if it hangs

// USER CONFIG: stepper motor config
// FYI: you can't set more than 8 motors without changing the directions stuff
int PIN_MAP[] = {5, 7}; // pwm pins that move each stepper motor (driver)
int PIN_DIR_MAP[] = {4, 6}; // pins that set direction for each stepper motor driver
int MICROSTEP_PIN_MAP[] = {11, 12, 13}; // pins that set same microstepping across all stepper drivers

// derived stepper config
const int NUM_MOTORS = 2;//sizeof(PIN_MAP) / sizeof(PIN_MAP[0]);
// reset arduino after 8 seconds.
const unsigned long WDT_DELAY = 8000000;  // consider this non-configurable


void fail(String msg="<unknown error>") {
  Serial.println("Fail: " + msg);
  // force hang & reset
  wdt_enable(WDTO_15MS);
  while (1) { }
}


void set_num_steps_per_turn() {
  Serial.println("Please pass the number of microsteps per turn: 0, 4, 8, 16, 32");
  while (!Serial.available()) {}
  int byte1 = Serial.read();
  switch (byte1) {
    case 0:
      Serial.println("Full Step (no microstepping)");
      digitalWrite(MICROSTEP_PIN_MAP[0], LOW);
      digitalWrite(MICROSTEP_PIN_MAP[1], LOW);
      digitalWrite(MICROSTEP_PIN_MAP[2], LOW);
      break;
    case 2:
      Serial.println("1/2 Step Microstepping");
      digitalWrite(MICROSTEP_PIN_MAP[0], HIGH);
      digitalWrite(MICROSTEP_PIN_MAP[1], LOW);
      digitalWrite(MICROSTEP_PIN_MAP[2], LOW);
      break;
    case 4:
      Serial.println("1/4 Step Microstepping");
      digitalWrite(MICROSTEP_PIN_MAP[0], LOW);
      digitalWrite(MICROSTEP_PIN_MAP[1], HIGH);
      digitalWrite(MICROSTEP_PIN_MAP[2], LOW);
      break;
    case 8:
      Serial.println("1/8 Step Microstepping");
      digitalWrite(MICROSTEP_PIN_MAP[0], HIGH);
      digitalWrite(MICROSTEP_PIN_MAP[1], HIGH);
      digitalWrite(MICROSTEP_PIN_MAP[2], LOW);
      break;
    case 16:
      Serial.println("1/16 Step Microstepping");
      digitalWrite(MICROSTEP_PIN_MAP[0], LOW);
      digitalWrite(MICROSTEP_PIN_MAP[1], LOW);
      digitalWrite(MICROSTEP_PIN_MAP[2], HIGH);
      break;
    case 32:
      Serial.println("1/32 Step Microstepping");
      digitalWrite(MICROSTEP_PIN_MAP[0], HIGH);
      digitalWrite(MICROSTEP_PIN_MAP[1], HIGH);
      digitalWrite(MICROSTEP_PIN_MAP[2], HIGH);
      break;
    default:
      fail(String ("You passed an invalid byte: ") + (int) byte1 +
           ". You must supply a valid number of stepper motor microsteps: " +
           " 0, 2, 4, 8, 16 or 32");
      break;
  }
}


void setup() {
  // watchdog: reset after X seconds if counter not reset
  wdt_enable(WDTO_8S);

  // serial stuff
  Serial.begin(9600); 
  while (!Serial) {
    ; // wait for serial port to connect. Needed for Leonardo only
  }
  Serial.println("Hello!");

  for (int i=0 ; i<NUM_MOTORS; i++) {
    pinMode(PIN_MAP[i], OUTPUT);
    pinMode(PIN_DIR_MAP[i], OUTPUT);
  }
  for (int i=0; i<3; i++) {
    pinMode(MICROSTEP_PIN_MAP[i], OUTPUT);
  }

  set_num_steps_per_turn();
  Serial.println(String ("\nYou now control ") + NUM_MOTORS + " stepper"
                 " motors.  Please pass " + (NUM_MOTORS * 4 + 4 + 1) +
                 " bytes at a time in Big Endian order.\n\n");
  Serial.println((String ("Each message must contain:")) +
                 "\n- per motor, a 4 byte int defining num steps to move" +
                 "\n- a 4 byte int defining microseconds to block for" +
                 "\n- 1 byte encoding the direction of each motor\n\n");
}


void loop() {
  wdt_reset (); // reset watchdog counter

  if (Serial.available()) {
    // read data from serial
    unsigned long step_pins[NUM_MOTORS];
    for (int i=0; i<NUM_MOTORS; i++) {
      step_pins[i] = serial_read_long();
    }
    unsigned long microsecs = serial_read_long();
    byte directions = serial_read_byte();

    // do stuff with gathered data
    Serial.println(((String) "stepping for ") + microsecs + " microsecs...");
    set_direction(directions);
    step(step_pins, microsecs);
  }
}

byte serial_read_byte() {
  /* read a byte from Serial */
  while (!Serial.available()) {}
  return Serial.read();
}

unsigned long serial_read_long() {
  /* read a 4 byte unsigned int (ie an unsigned long) from Serial */
  unsigned long a = 0;
  unsigned long b;
  for (int i=3 ; i>=0 ; i--) {
    b = serial_read_byte();
    a |= (b << (i*8));
  }
  return a;
}

void set_direction(byte directions) {
  /* Set the direction pin for each motor according to a bit sequence
  Big Endian Order: the first bit corresponds to the first motor

  `directions` is a 1-byte bitmap for recognized motors.
  */
  for (int jth_bit=0; jth_bit<8; jth_bit++) {
    if (directions & (1<<(7-jth_bit))) {
      digitalWrite(PIN_DIR_MAP[jth_bit], HIGH);
    } else {
      digitalWrite(PIN_DIR_MAP[jth_bit], LOW);
    }
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
  unsigned long total_num_steps = lcm(steps_per_pin, NUM_MOTORS);
  unsigned int delay_between_steps = microsecs / total_num_steps - 1;

  if (delay_between_steps > WDT_DELAY) {
    fail("step(): Too much delay between motor pulses. Try increasing the"
         " steps_per_pin values or reducing the total travel time");
  } else if (delay_between_steps < 1) {
    fail("step(): Not enough delay between motor pulses.  Try decreasing the"
         " steps_per_pin values or increasing the total travel time");
  }
  // for each pin, a pulse comes every cnt steps
  unsigned int counter_max[NUM_MOTORS];
  unsigned int counters[NUM_MOTORS];
  for (int i=0; i<NUM_MOTORS ; i++) {
    counter_max[i] = total_num_steps / steps_per_pin[i];
    counters[i] = 0; // hack: can't figur eout how to initialize properly
  }
  for (int step=0; step<=total_num_steps; step++) {
    // pulse the pins on each step
    for (int j=0; j < NUM_MOTORS ; j++) {
      if (++counters[j] >= counter_max[j]) {
        digitalWrite(PIN_MAP[j], HIGH);
      }
      delayMicroseconds(1);
      if (counters[j] >= counter_max[j]) {
        counters[j] = 0;
        digitalWrite(PIN_MAP[j], LOW);
      }
    }
    delayMicroseconds(delay_between_steps);
    wdt_reset(); // reset watchdog counter
  }
}


unsigned int gcd(unsigned int a, unsigned int b) {
  /* Find the Greatest Common Divisor of two ints */
  unsigned int t;
  while (b != 0) {
    t = a % b;
    a = b;
    b = t;
  }
  return a;
}


unsigned long lcm(unsigned long ints[], int num_ints) {
  /* Find the Lowest Common Multiple of an array of ints
   *
   * lcm = a*b / gcd(a, b)
   * lcm_of_many_ints = reduce(lcm, ints)
   */
  unsigned long lcm_val = ints[0];
  for (int i=1 ; i < num_ints ; i++) {
    lcm_val = lcm_val * (ints[i] / gcd(lcm_val, ints[i]));
  }
  return lcm_val;
}
