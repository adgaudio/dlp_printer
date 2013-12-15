#include "avr/wdt.h"  // watchdog resets avr chip if it hangs
#include "Firmata.h"  // protocol library for communicating with software
#include "Custom_pwm.h" // expose code to change max pwm frequency on pins

const int x_step_pin = 5;
const int y_step_pin = 6;
const int x_dir_pin = 4;

void setup() {
  // watchdog: reset after two seconds if counter not reset
  wdt_enable (WDTO_2S); 
  
  // firmata
  Firmata.setFirmwareVersion(0, 1);
  Firmata.attach(ANALOG_MESSAGE, firmata_callback);
  Firmata.begin();
  
  // serial stuff
  Serial.begin(9600); 
  while (!Serial) {
    ; // wait for serial port to connect. Needed for Leonardo only
  }
    
  pinMode(x_step_pin, OUTPUT);
  pinMode(x_dir_pin, OUTPUT);

  // TEMP TESTING
  CustomPwm cust;
  cust.setPwmFrequency(x_step_pin, 1); // this would be for laser galvos
}

void loop() {
  Serial.println(".top of loop"); // debug
  wdt_reset (); // reset watchdog counter
  
  while(Firmata.available()) {
    Firmata.processInput();
  }
}

void step(int step_pins[], int len_step_pins) {
  /* Given a group of pins, 
  pulse all of them high and then all of them low.
  
  Group pin writes together so a delay only happens once
  */
  for (int i=0; i < len_step_pins ; i++) {
    digitalWrite(step_pins[i], HIGH);
  }
  delay(1);
  for (int i=0; i < len_step_pins ; i++) {
    digitalWrite(step_pins[i], LOW);
  }
  delay(0);
}

void firmata_callback(byte pin, int value) {
  Serial.println("firmata callback");
  Serial.print(pin);
  Serial.print(value);
  Serial.println("...");
    
  int step_pins[] = {x_step_pin};
  int len_step_pins = sizeof(step_pins) / sizeof(int);
  step(step_pins, len_step_pins);
}

