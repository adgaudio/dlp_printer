#ifndef __MOTORS_H_INCLUDED__
#define __MOTORS_H_INCLUDED__

#include "util.h"
#include "configuration.h"

namespace motor {
  extern void configure();
  extern void set_num_steps_per_turn(int microsteps);
  extern void set_direction(int motor_idx, bool forward);
  extern void slide_vat(int motor_idx, unsigned long nsteps, unsigned long feedrate);
}
#endif  // __MOTORS_H_INCLUDED__
