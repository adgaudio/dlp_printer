#ifndef __MOTORS_H_INCLUDED__
#define __MOTORS_H_INCLUDED__

#include "configuration.h"
#include "Arduino.h"
#include "util.h"

namespace motor {
  void configure();
  void set_num_steps_per_turn();
  void set_direction(byte directions);
}
#endif  // __MOTORS_H_INCLUDED__
