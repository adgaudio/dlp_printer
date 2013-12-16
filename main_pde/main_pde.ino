#include "avr/wdt.h"  // watchdog resets avr chip if it hangs
#include "Firmata.h"  // protocol library for communicating with software
#include "Custom_pwm.h" // expose code to change max pwm frequency on pins

const int x_step_pin = 5;
const int y_step_pin = 6;
const int x_dir_pin = 4;
const int X_STEPS_PER_REV = 200 * 32;

const int pwm_divisor = 8; // arduino default = 64
CustomPwm cust;

void setup() {
  // watchdog: reset after two seconds if counter not reset
  wdt_enable (WDTO_2S); 
  
  // firmata
  //Firmata.setFirmwareVersion(0, 1);
  //Firmata.attach(ANALOG_MESSAGE, analogWriteCallback);
  //Firmata.begin();
  
  // serial stuff
  Serial.begin(9600); 
  while (!Serial) {
    ; // wait for serial port to connect. Needed for Leonardo only
  }
    
  pinMode(x_step_pin, OUTPUT);
  pinMode(x_dir_pin, OUTPUT);

  // Make the timer run faster
  cust.setPwmFrequency(x_step_pin, 8); // this would be for laser galvos
}

void loop() {
  wdt_reset (); // reset watchdog counter
  
  //while(Firmata.available()) {
  //  Firmata.processInput();
  //}
  int step_pins[] = {x_step_pin};
  step(step_pins, 200*16, 0);
  cust.wait(1000);
}

void step(int step_pins[], int num_steps, float millis_per_step) {
  /* Given a group of pins, 
  pulse all of them high and then all of them low.
  
  Group pin writes together so a delay only happens once
  */
  int num_step_pins = sizeof(step_pins) / sizeof(int);

  for (int k=0 ; k < num_steps ; k++) { // TODO: declare num steps per turn elsewhere!
  for (int i=0; i < num_step_pins ; i++) {
    digitalWrite(step_pins[i], HIGH);
  }
  cust.wait(millis_per_step/2.0);

  for (int i=0; i < num_step_pins ; i++) {
    digitalWrite(step_pins[i], LOW);
  }
  cust.wait(millis_per_step/2.0);
  wdt_reset (); // reset watchdog counter

  }
}

void analogWriteCallback(byte pin, int value) {
  Serial.println("firmata callback");
  Serial.print(pin);
  Serial.print(value);
  Serial.println("...");

  if (IS_PIN_PWM(pin)) {
    pinMode(PIN_TO_DIGITAL(pin), OUTPUT);
    analogWrite(PIN_TO_PWM(pin), value);
  }
}

