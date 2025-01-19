#include "motor.h"

Motor::Motor()
{
  enc = nullptr;
}

void Motor::begin()
{
  enc = new QuadEncoder(MOTOR::ENC_CHA, MOTOR::ENC_PIN_A, MOTOR::ENC_PIN_B, MOTOR::ENC_PULL_UP);
  enc->setInitConfig();
  enc->EncConfig.filterCount = MOTOR::ENC_FILTER_N;
  enc->EncConfig.filterSamplePeriod = MOTOR::ENC_FILTER_T_US;
  enc->init();
  enc->write(0);
  pinMode(MOTOR::PWM_PIN, OUTPUT);
  pinMode(MOTOR::DIR_PIN, OUTPUT);
  digitalWriteFast(MOTOR::DIR_PIN, LOW);
  analogWriteFrequency(MOTOR::PWM_PIN,MOTOR::PWM_FREQ);
  analogWriteResolution(MOTOR::PWM_RES);
  mot_vol = 0.0f;
  mot_pos = 0.0f;
  mot_vel = 0.0f;
}

uint32_t Motor::write_voltage(float u)
{
  uint32_t pwm_bits = 0;
  // Limit floating point value 
  // TODO: Llimit either bits or float for performrnace
  if(u > MOTOR::U_MAX)
  {
    u = MOTOR::U_MAX;
  }else if(u < -1.0f*MOTOR::U_MAX)
  {
    u = -1.0f*MOTOR::U_MAX;
  }
  pwm_bits = (uint32_t)(fabs(u)*(float)(MOTOR::PWM_MAX)/MOTOR::U_MAX);
  if(pwm_bits > MOTOR::PWM_MAX)
  {
    pwm_bits = MOTOR::PWM_MAX;
  }else if(pwm_bits < MOTOR::PWM_MIN)
  {
    pwm_bits = MOTOR::PWM_MIN;
  }
  digitalWriteFast(MOTOR::DIR_PIN, u >= 0);
  analogWrite(MOTOR::PWM_PIN, pwm_bits);
  mot_vol = u;
  return pwm_bits;
}

void Motor::clear_count()
{
  enc->write(0);
}

void Motor::clear()
{
  write_voltage(0.0f);
  clear_count();
  mot_vol = 0.0f;
  mot_pos = 0.0f;
  mot_vel = 0.0f;
}
float Motor::read_position()
{
  return mot_pos = (float) (enc->read()*MOTOR::ENC_CNT_2_RAD);
}
float Motor::read_velocity()
{
  return mot_pos = (float) (enc->read()*MOTOR::ENC_CNT_2_RAD);
}
float Motor::read_voltage()
{
  return mot_vol;
}
int32_t Motor::read_count()
{
  return enc->read();
}