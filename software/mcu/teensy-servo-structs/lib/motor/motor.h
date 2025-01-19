#ifndef MOTOR_H
#define MOTOR_H
#include <Arduino.h>
#include "QuadEncoder.h"
#include <math.h>
namespace MOTOR
{
  constexpr uint8_t  PWM_PIN     = 11;
  constexpr uint8_t  DIR_PIN     = 12;
  constexpr uint8_t  CS_PIN      = 41;  // A17
  constexpr uint16_t PWM_RES     = 11;
  constexpr uint32_t PWM_MAX     = (2 << (PWM_RES-1))-1;
  constexpr uint32_t PWM_MIN     = 0;
  constexpr float    PWM_FREQ    = 73242.19f;
  constexpr float    U_MAX       = 12.16f;
  constexpr uint8_t  ENC_CHA     = 1;
  constexpr uint8_t  ENC_PIN_A   = 0;
  constexpr uint8_t  ENC_PIN_B   = 1;
  constexpr uint8_t  ENC_PULL_UP = 0;
  constexpr uint8_t  ENC_FILTER_N = 5;
  constexpr uint8_t  ENC_FILTER_T_US = 5;
  constexpr uint32_t  ENC_PPR  = 5120;
  constexpr uint32_t  ENC_CPPR = 4*5120;
  constexpr float     ENC_CNT_2_RAD = 2.0*M_PI/((float)ENC_CPPR);
};

class Motor
{
  private:
    QuadEncoder *enc;
    float mot_vol;
    float mot_pos;
    float mot_vel;
  public:
    Motor();
    void begin();
    void clear();
    void clear_count();
    uint32_t write_voltage(float u);
    float read_position();
    float read_velocity();
    float read_voltage();
    int32_t read_count();

};


#endif

