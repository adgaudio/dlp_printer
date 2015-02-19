This repo contains all code I use to build a Laser-scanning SLA 3D printer.
This is a complicated and fun ongoing project!  I will add more
documentation on this project soon, so keep checking in or send me a
message!

The main components are:

- The mechanics and design are done in openscad.  One can build all plastic
  parts on any standard FFM printer.
- The Firmware fits on an Arduino UNO.  I'm using some peripheral hardware
  not well explained here.  (DACs + Laser Galvanometer Drivers from
  Ebay, Pololu Stepper Drivers, diy ttl laser power source).  I'll
  eventually document this.
- The software that interfaces with this hardware runs on node.js.  A
  node.js app reads gcode (such as that output by Slic3r) and then sends
  it to the printer via Serial port.
