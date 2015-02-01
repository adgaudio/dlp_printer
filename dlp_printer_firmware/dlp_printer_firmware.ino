#include "avr/wdt.h"  // watchdog resets avr chip if it hangs
#include "configuration.h"

// This firmware runs on an arduino and facilitates communication between a
// computer and a dlp printer(containing stepper motors and a laser scanning
// galvanometer).


// reset arduino after 8 seconds.
const unsigned long WDT_DELAY = 8000000;  // consider this non-configurable


// force hang & reset
void fail(String msg="<unknown error>") {
  Serial.println("Fail: " + msg);
  wdt_enable(WDTO_15MS);
  while (1) { }
}


// Configure stepper motor microstepping
void set_num_steps_per_turn() {
  Serial.println("Please pass exactly 1 byte specifying the number of microsteps per turn: 0, 4, 8, 16, 32");
  int byte1 = (int) serial_read_byte();
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
      fail(String ("You passed an invalid byte: ") + (int) byte1 +
           ". You must supply a valid number of stepper motor microsteps: " +
           " 0, 2, 4, 8, 16 or 32");
      break;
  }
}


void configure_motors() {
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


void configure_lasers() {
  /* Initial settings for Laser pins */
  // TODO
  for (int i=0; i<2; i++) {
    pinMode(LASER_XY_PINS[i], OUTPUT);
  }
  pinMode(LASER_POWER_PIN, OUTPUT);
  digitalWrite(LASER_POWER_PIN, LOW);  // off by default
}


void setup() {
  // watchdog: reset after X seconds if counter not reset
  wdt_enable(WDTO_8S);  // TODO: is WDTO_8S the same as WDT_DELAY?

  // serial stuff
  Serial.begin(BAUDRATE);
  while (!Serial) {
    ; // wait for serial port to connect. Needed for Leonardo only
  }
  Serial.println("Hello!");
  configure_motors();
  configure_lasers();

  Serial.println(String ("\nYou now control ") + NUM_MOTORS + " stepper"
                 " motors and " + NUM_LASER_MOTORS + "lasers.  Please pass "
                 + (NUM_MOTORS * 4 + 4 + 1) +
                 " bytes at a time in Big Endian order.\n\n");  // TODO: figure out how to drive laser motors and define instructions here
  Serial.println((String ("Each message must contain:")) +
      "\n- per motor, a (4 byte) long defining num steps to move" +
      "\n- a (4 byte) long defining microseconds to block for" +
      "\n- 1 byte bitmap encoding the respective direction of each motor\n\n");
  Serial.flush();
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
      digitalWrite(MOTOR_DIR_PINS[jth_bit], HIGH);
    } else {
      digitalWrite(MOTOR_DIR_PINS[jth_bit], LOW);
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
  unsigned long delay_between_steps = microsecs / total_num_steps - 2;

  if (delay_between_steps > WDT_DELAY) {
    fail("step(): Too much delay between motor pulses. Try increasing the"
         " steps_per_pin values or reducing the total travel time");
  } else if (delay_between_steps < 2) {
    fail("step(): Not enough delay between motor pulses.  Try decreasing the"
         " steps_per_pin values or increasing the total travel time");
  }
  // for each pin, a pulse comes every cnt steps
  unsigned int counter_max[NUM_MOTORS];
  unsigned int counters[NUM_MOTORS];
  for (int i=0; i<NUM_MOTORS ; i++) {
    counter_max[i] = total_num_steps / steps_per_pin[i];
    counters[i] = 0;
  }
  for (long step=0; step<=total_num_steps; step++) {
    // pulse the pins on each step
    for (int j=0; j < NUM_MOTORS ; j++) {
      if (++counters[j] >= counter_max[j]) {
        digitalWrite(MOTOR_STEP_PINS[j], HIGH);
      }
    }
    delayMicroseconds(2);
    for (int j=0; j < NUM_MOTORS ; j++) {
      if (counters[j] >= counter_max[j]) {
        counters[j] = 0;
        digitalWrite(MOTOR_STEP_PINS[j], LOW);
      }
    }
    delayMicroseconds(delay_between_steps);
    wdt_reset(); // reset watchdog counter
  }
}


unsigned int gcd(unsigned long a, unsigned long b) {
  /* Find the Greatest Common Divisor of two ints */
  unsigned long t;
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
