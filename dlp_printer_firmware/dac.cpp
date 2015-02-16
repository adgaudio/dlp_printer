// Control a DAC MCP4725
// Great instructions from Adafruit!
// https://github.com/adafruit/Adafruit_MCP4725/blob/master/Adafruit_MCP4725.cpp
//
#include "dac.h"

namespace dac {

  // Define the current phase (ie position) in the analog wave we're outputting
  // 2048 is half of 4096, which matches the chip's default starting position
  // at mid-voltage
  // uint16_t PHASE = 0x800  // 2048

  /*
   * testing...
   * uint8_t i = 0;
   * void setup() { Wire.begin(); }
   * void loop() {
   *   set_voltage(i, DEVICE_ADDR);
   *   i = (i + 1) % 4096
   * }
   */
}
