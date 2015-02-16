#include "laser.h"

namespace laser {
  // Starting position of the laser galvos.  {xpos, ypos}
  // The chip is pre-programmed to dead center
  uint16_t PHASE[] = {0x800, 0x800};  // 2048


  void configure() {
    /* Initial settings for Laser pins */
    // TODO
    for (int i=0; i<2; i++) {
      pinMode(LASER_XY_PINS[i], OUTPUT);
    }
    // ensure laser is powered off
    pinMode(LASER_POWER_PIN, OUTPUT);
    digitalWrite(LASER_POWER_PIN, LOW);  // off by default
    // power on laser galvanometers
    pinMode(LASER_GALVO_POWER_PIN, OUTPUT);
    digitalWrite(LASER_GALVO_POWER_PIN, HIGH);
  }


  // private func:
  void set_voltage(uint16_t output, uint8_t device_addr) {
    /* Set the output voltage on a Digital to Analoc Converter: DAC MCP4725
     * Great instructions from Adafruit!
     * github.com/adafruit/Adafruit_MCP4725/blob/master/Adafruit_MCP4725.cpp
     *
     * `output` - value from 0 to 4095
     * `device_addr` the I2C address of the chip
     */
    uint8_t twbrback = TWBR;
    TWBR = ((F_CPU / 400000L) - 16) / 2; // Set I2C frequency to 400kHz
    Wire.beginTransmission(device_addr);

    // either write to EEPROM to store some state (persists through power off)
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


  void step(int x_or_y, bool forward) {
    /* Move the x or y laser galvo forward or backward one step.*/
    PHASE[x_or_y] += forward;
    set_voltage(PHASE[x_or_y], DEVICE_ADDRS[x_or_y]);
  }
}
