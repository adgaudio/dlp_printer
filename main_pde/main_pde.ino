#include "avr/wdt.h"  // watchdog resets avr chip if it hangs

// stepper motor config
const int _X_DIR_PIN = 4;
const int _X_STEP_PIN = 5;
const int _Y_DIR_PIN = 6;
const int _Y_STEP_PIN = 7;
const int NUM_PINS = 2; //sizeof(PIN_MAP) / sizeof(PIN_MAP[0]);
const int X_STEPS_PER_REV = 200 * 32;
// derived stepper config
int PIN_MAP[] = {_X_STEP_PIN, _Y_STEP_PIN}; // list the pwm pins for each stepper motor
int PIN_DIR_MAP[] = {_X_DIR_PIN, _Y_DIR_PIN}; // list the dir pins for each stepper motor
int MICROSTEP_PIN_MAP = {11, 12, 13};

// reset arduino after 8 seconds.
const double WDT_DELAY = 8000000;  // consider this non-configurable


void fail(String msg="<unknown error>") {
  Serial.println("Fail: " + msg);
  // force hang
  while (1) { }
}


void setup() {
  // watchdog: reset after X seconds if counter not reset
  wdt_enable(WDTO_8S);

  // serial stuff
  Serial.begin(9600); 
  //while (!Serial) {
  //  ; // wait for serial port to connect. Needed for Leonardo only
  //}
  Serial.println("Hello!");

  for (int i=0 ; i<NUM_PINS; i++) {
    pinMode(PIN_MAP[i], OUTPUT);
    pinMode(PIN_DIR_MAP[i], OUTPUT);
  }
  set_num_steps_per_turn()
}

void set_num_steps_per_turn() {
  Serial.println("Please pass the number of microsteps per turn: 0, 4, 8, 16, 32");
  byte1 = Serial.read();
  switch (byte1) {
    case 0:
      // Full Step (no microstepping)
      pinMode(MICROSTEP_PIN_MAP[0], LOW);
      pinMode(MICROSTEP_PIN_MAP[1], LOW);
      pinMode(MICROSTEP_PIN_MAP[2], LOW);
    case 2:
      // 1/2 Step Microstepping
      pinMode(MICROSTEP_PIN_MAP[0], HIGH);
      pinMode(MICROSTEP_PIN_MAP[1], LOW);
      pinMode(MICROSTEP_PIN_MAP[2], LOW);
    case 4:
      // 1/4 Step Microstepping
      pinMode(MICROSTEP_PIN_MAP[0], LOW);
      pinMode(MICROSTEP_PIN_MAP[1], HIGH);
      pinMode(MICROSTEP_PIN_MAP[2], LOW);
    case 8:
      // 1/8 Step Microstepping
      pinMode(MICROSTEP_PIN_MAP[0], HIGH);
      pinMode(MICROSTEP_PIN_MAP[1], HIGH);
      pinMode(MICROSTEP_PIN_MAP[2], LOW);
    case 16:
      // 1/16 Step Microstepping
      pinMode(MICROSTEP_PIN_MAP[0], LOW);
      pinMode(MICROSTEP_PIN_MAP[1], LOW);
      pinMode(MICROSTEP_PIN_MAP[2], HIGH);
    case 32:
      // 1/32 Step Microstepping
      pinMode(MICROSTEP_PIN_MAP[0], HIGH);
      pinMode(MICROSTEP_PIN_MAP[1], HIGH);
      pinMode(MICROSTEP_PIN_MAP[2], HIGH);
    default:
      fail("You passed an invalid byte.  You must supply a valid number of stepper motor microsteps: 0, 2, 4, 8, 16 or 32");
  }
}

void loop() {
  wdt_reset (); // reset watchdog counter
  // TODO: get step from Serial
  // TODO: test moving motor up and down
  // TODO: make a function that moves the vat and then raises the platform
  // TODO: try another function that constantly raises the platform
  double step_pins[] = {6400, 10};
  step(step_pins, 7000000);
  delay(100);
}

void step(double steps_per_pin[], double microsecs) {
  /*
  Given a number of steps per pin,
  For each pin,
  pulse a high-low sequence once per step over the course of X `microsecs`

  Pulses happen as close to concurrently as possible, and pins are identified
  by index in the array
  */
  double total_num_steps = lcm(steps_per_pin, NUM_PINS);

  unsigned int delay_between_steps = microsecs / total_num_steps - 1;
  if (delay_between_steps > WDT_DELAY) {
    fail("step(): Too much delay between motor pulses. Try increasing the"
         " steps_per_pin values or reducing the total travel time");
  } else if (delay_between_steps < 1) {
    fail("step(): Not enough delay between motor pulses.  Try decreasing the"
         " steps_per_pin values or increasing the total travel time");
  }
  // for each pin, a pulse comes every cnt steps
  unsigned int counter_max[NUM_PINS];
  unsigned int counters[NUM_PINS];
  for (int i=0; i<NUM_PINS ; i++) {
    counter_max[i] = total_num_steps / steps_per_pin[i];
    counters[i] = 0; // hack: can't figur eout how to initialize properly
  }
  for (int step=0; step<=total_num_steps; step++) {
    // pulse the pins on each step
    for (int j=0; j < NUM_PINS ; j++) {
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


double lcm(double ints[], int num_ints) {
  /* Find the Lowest Common Multiple of an array of ints
   *
   * lcm = a*b / gcd(a, b)
   * lcm_of_many_ints = reduce(lcm, ints)
   */
  double lcm_val = ints[0];
  for (int i=1 ; i < num_ints ; i++) {
    lcm_val = lcm_val * (ints[i] / gcd(lcm_val, ints[i]));
  }
  return lcm_val;
}
