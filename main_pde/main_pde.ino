#include "avr/wdt.h"  // watchdog resets avr chip if it hangs
#include "Custom_pwm.h" // expose code to change max pwm frequency on pins

const int X_STEP_PIN = 5;
const int Y_STEP_PIN = 6;
const int X_DIR_PIN = 4;
const int X_STEPS_PER_REV = 200 * 32;
const int PIN_MAP = {X_STEP_PIN, Y_STEP_PIN};
const int NUM_PINS = sizeof(PIN_MAP) / sizeof(PIN_MAP[0]);
const int PWM_DIVISOR = 8; // arduino default = 64
CustomPwm custpwm;


void fail(msg="<unknown error>") {
  Serial.println("Fail: " + msg)
  // force hang.
  while (1) { }
}


void setup() {
  // watchdog: reset after X seconds if counter not reset
  // --> because we're setting a custom pwm, this shortens the delay a lot!
  wdt_enable (WDTO_5S);

  // serial stuff
  Serial.begin(9600); 
  /*while (!Serial) {*/
    /*; // wait for serial port to connect. Needed for Leonardo only*/
  /*}*/

  pinMode(X_STEP_PIN, OUTPUT);
  pinMode(X_DIR_PIN, OUTPUT);

  // Make the timer run faster
  custpwm.setPwmFrequency(X_STEP_PIN, PWM_DIVISOR);
}


void loop() {
  wdt_reset (); // reset watchdog counter

  int step_pins[] = PIN_MAP;
  step(step_pins, 1000);
  custpwm.wait(1000);
}


void step(int[NUM_PINS] steps_per_pin, int millisecs) {
  /*
  Given a number of steps per pin,
  For each pin,
  pulse a high-low sequence once per step over the course of X `millisecs`

  Pulses happen as close to concurrently as possible, and pins are identified
  by index in the array
  */
  int total_num_steps = lcm(steps_per_pin, NUM_PINS);

  float delay_between_steps = (float)millisecs / (float)total_num_steps;
  if (delay_between_steps > WDTO_5S) {
    fail("step(): Too much delay between motor pulses. Try increasing the" +
         " steps_per_pin values or reducing the total travel time");
  }
  // for each pin, a pulse comes every cnt steps
  int counter_max[NUM_PINS];
  int counters[NUM_PINS] = {0};
  for (int i=0; i<NUM_PINS ; i++) {
    counter_max[i] = total_num_steps / steps_per_pin[i];
  }

  for (int step=0; step<=total_num_steps; step++) {

    // pulse the pins on each step
    for (int j=0; j<= NUM_PINS ; j++) {
      if (++counters[j] >= counter_max[j]) {
        counters[j] = 0;
        digitalWrite(PIN_MAP[j], HIGH);
        digitalWrite(PIN_MAP[j], LOW);
      }
      // hack: might need this to ensure it actually registers as a pulse
      //custpwm.wait(0); // hack: I think this actually does wait a little
      //if (counters[j] >= counter_max[j]) {
        //counters[j] = 0;
        //digitalWrite(PIN_MAP[j], LOW);
      //}
    }
    custpwm.wait(delay_between_steps);
    wdt_reset(); // reset watchdog counter
  }
}


int gcd(int a, int b) {
  /* Find the Greatest Common Divisor of two ints */
  int t;
  while (b != 0) {
    t = a % b;
    a = b;
    b = t;
  }
  return a;
}


int lcm(int ints[], int num_ints) {
  /* Find the Lowest Common Multiple of an array of ints
   *
   * lcm = a*b / gcd(a, b)
   * lcm_of_many_ints = reduce(lcm, ints)
   */
  int lcm_val = ints[0];
  for (int i=1 ; i < num_ints ; i++) {
    lcm_val = lcm_val * ints[i] / gcd(lcm_val, ints[i]);
  }
  return lcm_val;
}
