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
#define ARDUINO_DEFAULT_PWM_DIVISOR 64

#include <Arduino.h>

class CustomPwm {
  public:
    void setPwmFrequency(int pin, int divisor);
    void wait(float millisecs);
  private:
    int current_divisor;
};
#endif
