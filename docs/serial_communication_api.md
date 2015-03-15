This file explains how to communicate with the printer firmware.


The firmware for this printer does not recognize G-Code.  However, the node.js app reads gcode and then converts the gcode into the following format, which is suitable for the printer hardware.

Steps:

1. Connect printer's Arduino to computer via USB-Serial port.
1. Arduino starts up, configures itself, and sends message: "Hello!"
1. Computer sends messages to Arduino instructing it what to do.  All
   messages sent to the Arduino should have the following format:

  - Please encode ints in Big Endian order unless otherwise
    specified.
  - All messages must start with the following prefix
    - 1 byte:
      (1 bit: update configuration 1=yes|0=no)  // TODO. ignored.
      (1 bit: write configuration to EEPROM 1=yes|0=no)  // TODO.  ignored
      (1 bit: empty)
      (1 bit: motor power on=1|off=0)  // 1<<4
      (1 bit: laser galvos power on=1|off=0)  // 1<<3
      (1 bit: laser power on=1|off=0)  // 1<<2
      (1 bit: move motors? 1=yes|0=no)  // 1<<1
      (1 bit: move laser galvos? 1=yes|0=no)  // 1

  - If move motors == yes or move lasers == yes, message must
    proceed with:
    - (int_32: feedrate, or the "steps per microsecond" speed to use)
    - 1 byte specifying motor directions:
      (4 bits: empty)
      (1 bit: laser_galvo_x dir 1=forward|0=backward)  // 1<<3
      (1 bit: laser_galvo_y dir 1=forward|0=backward)  // 1<<2
      (1 bit: motor_0 dir 1=forward|0=backward)  // 1<<1
      (1 bit: motor_1 dir 1=forward|0=backward)  // 1  ... aka M100, or slide vat side to side.
  - If move motors == yes, message must proceed with:
    - (int_32: num steps motor 0)  // z axis
    - (int_32: num steps motor 1)  // moves the vat
  - If move laser galvos, the message must proceed with:
    - (int_12: x position)
    - (int_12: y position)

Happy printing!
