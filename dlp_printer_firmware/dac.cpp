// Control a DAC MCP4725
// Great instructions from Adafruit!
// https://github.com/adafruit/Adafruit_MCP4725/blob/master/Adafruit_MCP4725.cpp
//
#include <Wire.h>

// temp testing config
uint8_t DEVICE_ADDR = 0x60
    // For MCP4725[A0] the address is 0x60 or 0x61  # odd num has ADDR tied to VCC
    // For MCP4725[A1] the address is 0x62 or 0x63
    // For MCP4725[A2] the address is 0x64 or 0x65


void set_voltage(uint16_t output, uint8_t device_addr) {
  /* Set the output voltage on the DAC MCP4725
   *
   * output - value from 0 to 4095
   */
  uint8_t twbrback = TWBR;
  TWBR = ((F_CPU / 400000L) - 16) / 2; // Set I2C frequency to 400kHz

  Wire.beginTransmission(device_addr);

  // either write to EEPROM to store some state that persists through power off
  // Wire.write(0x60);
  // or just write to DAC register:
  Wire.write(0x40);

  // Send 2 bytes of data, 1 byte at a time
  // Upper data bits          (D11.D10.D9.D8.D7.D6.D5.D4)
  // Lower data bits          (D3.D2.D1.D0.x.x.x.x)
  Wire.write(output >> 4);  // write 1 byte
  Wire.write((output % 16) << 4);
  Wire.endTransmission();

  TWBR = twbrback;
}


// Define the current phase (ie position) in the analog wave we're outputting
// 2048 is half of 4096, which matches the chip's default starting position at mid-voltage
// uint16_t PHASE = 0x800  // 2048

void step(unsigned long steps_per_pin[], unsigned long microsecs) {
  // Do I want to consider adjusting the laser position the same way I adjust motor positions?
  // Should laser + motor move at the same timestep?  or one and then the other?
  // ... TODO: tbd.  think about this.
}

/*
 * testing...
 * uint8_t i = 0;
 * void setup() { Wire.begin(); }
 * void loop() {
 *   set_voltage(i, DEVICE_ADDR);
 *   i = (i + 1) % 4096
 * }
 */
