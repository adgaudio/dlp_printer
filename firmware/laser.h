#ifndef __LASER_H_INCLUDED__
#define __LASER_H_INCLUDED__

#include "Arduino.h"
#include <Wire.h>

#include "configuration.h"
#include "util.h"

namespace laser {
  extern void configure();
  extern void step(int x_or_y, bool forward);
}
#endif  // __LASER_H_INCLUDED__
