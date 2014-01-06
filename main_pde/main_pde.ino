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
  Serial.println("Hello World");

  for (int i=0 ; i<NUM_PINS; i++) {
    pinMode(PIN_MAP[i], OUTPUT);
    pinMode(PIN_DIR_MAP[i], OUTPUT);
  }
}


void loop() {
  wdt_reset (); // reset watchdog counter

  double step_pins[] = {800, 400};
  step(step_pins, 1000000);
  delay(500);
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
