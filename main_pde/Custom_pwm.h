/*
 * Arduino library to set pwm frequency on specified pins much higher than
 * normal.  This may have unexpected results on time-dependent code.
 *
 * Adapted from:
 * https://github.com/nothinglabs/lasershow/blob/master/lasershow/lasershow.pde
 *
 */
#ifndef Custom_pwm_h
#define Custom_pwm_h

#include <Arduino.h>

class CustomPwm {
  public:
    void setPwmFrequency(int pin, int divisor);
};
#endif
