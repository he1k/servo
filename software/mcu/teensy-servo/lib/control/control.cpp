#include "control.h"
// Constructor
PID::PID()
{

}
// Set controller values
void PID::begin(float Ts, float Kp, float tau_i, float tau_d, float alpha, float lim, bool en_lim)
{
  this->_Ts = Ts;
  this->_Kp = Kp;
  this->_tau_i = tau_i;
  this->_tau_d = tau_d;
  this->_alpha = alpha;
  this->_a[0] = tau_i*(Ts + 2.0*alpha*tau_d);
  this->_b[0] = (0.5*Kp*(Ts + 2.0*tau_d)*(Ts + 2.0*tau_i)) / this->_a[0];
  this->_b[1] = (0.5*Kp*(Ts - 2.0*tau_d)*(Ts + 2.0*tau_i) + 0.5*Kp*(Ts + 2.0*tau_d)*(Ts - 2.0*tau_i)) / this->_a[0];
  this->_b[2] = (0.5*Kp*(Ts - 2.0*tau_d)*(Ts - 2.0*tau_i)) / this->_a[0];
  this->_a[1] = (tau_i*(Ts - 2.0*alpha*tau_d) - 1.0*tau_i*(Ts + 2.0*alpha*tau_d)) / this->_a[0];
  this->_a[2] = (-1.0*tau_i*(Ts - 2.0*alpha*tau_d)) / this->_a[0];
  this->_a[0] = 1; // Filter is normalized now, so set to 1
  this->_e[0] = 0;
  this->_e[1] = 0;
  this->_e[2] = 0;
  this->_u[0] = 0;
  this->_u[1] = 0;
  this->_u[2] = 0;
  this->_lim = lim;
  if(lim < 0) {
    this->_lim = fabs(this->_lim);
  }
  this->_en_lim = en_lim;
}
// Update controller
float PID::update(float r, float m)
{
  // Calculate e(t)
  this->_e[0] = r-m;
  // Calculate u(t) using direct implementation
  this->_u[0] = this->_b[0]*this->_e[0] + this->_b[1]*this->_e[1] + this->_b[2]*this->_e[2] - this->_a[1]*this->_u[1] - this->_a[2]*this->_u[2];
  // Clamp u(t)
  if(this->_en_lim)
  {
    if(this->_u[0] > this->_lim)
    {
      this->_u[0] = this->_lim;
    }else if(this->_u[0] < -this->_lim)
    {
      this->_u[0] = -this->_lim;
    }
  }
  // Update filter
  this->_u[2] = this->_u[1];
  this->_u[1] = this->_u[0];
  this->_e[2] = this->_e[1];
  this->_e[1] = this->_e[0];
  return this->_u[0];
}
// Reset controller
void PID::reset()
{
  for(uint8_t i = 0; i < 3; i++){
    this->_e[i] = 0;
    this->_u[i] = 0;
  }
}
// Get sampling time
double PID::getTs()
{
  return this->_Ts;
}
// Get proportional gain
double PID::getKp()
{
  return this->_Kp;
}
// Get integral time constant
double PID::getTaui()
{
  return this->_tau_i;
}
// Get differential time constant
double PID::getTaud()
{
  return this->_tau_d;
}
// Get feedback coeffients
double PID::getA(int idx)
{
  if((idx >= 0) && (idx <= 2))
    return this->_a[idx];
  else
    return OUT_OF_BOUNDS;
}
// Get feed forward coefficients
double PID::getB(int idx)
{
  if((idx >= 0) && (idx <= 2))
    return this->_b[idx];
  else
    return OUT_OF_BOUNDS;
}
// Test function with custom arguments
void PID::test(float Ts, float Kp, float tau_i, float tau_d, float alpha, float lim, bool en_lim, float e)
{
  this->begin(Ts, Kp, tau_i, tau_d, alpha, lim, en_lim);
  for(int i = 0; i < 10; i++)
  {
    Serial.print("u = "); Serial.println(this->_u[0],15);
    this->update(e, 0);
  }
  this->reset();
}
// Test function using values already present in object
void PID::test(float e)
{
  this->test(this->_Ts, this->_Kp, this->_tau_i, this->_tau_d, this->_alpha, this->_lim, this->_en_lim, e);
}
void PID::print()
{
  Serial.println("#######################################");
  Serial.println("##       PID Controller - Summary    ##");
  Serial.println("#######################################");
  Serial.println("\n## Parameters: ");
  Serial.print("Ts = ");     Serial.println(this->_Ts,9);
  Serial.print("Kp = ");     Serial.println(this->_Kp,9);
  Serial.print("tau_i = ");  Serial.println(this->_tau_i,9);
  Serial.print("tau_d = ");  Serial.println(this->_tau_d,9);
  Serial.print("alpha = ");  Serial.println(this->_alpha,9);
  Serial.print("en_lim = "); Serial.println(this->_en_lim,9);
  Serial.print("lim = ");    Serial.println(this->_lim,9);
  Serial.println("\n## Transfer function: ");
  Serial.print(this->_b[0],4); Serial.print(" + ");
  Serial.print(this->_b[1],4); Serial.print("z^(-1) + ");
  Serial.print(this->_b[2],4); Serial.print("z^(-2)");
  Serial.println();
  Serial.println("-------------------------------------");
  Serial.print(this->_a[0],4); Serial.print(" + ");
  Serial.print(this->_a[1],4); Serial.print("z^(-1) + ");
  Serial.print(this->_a[2],4); Serial.print("z^(-2)");
  Serial.println("\n");
  Serial.println("#######################################");
}
/*
float pictrl(float e, float Kp, float tau, float Ts){
  float b1 = Ts/(2*tau)+1;
  float b0 = Ts/(2*tau)-1;
  float a1 = 1;
  float a0 = -1;
  static float x = e, xp = e, y = 0, yp = 0;
  x = e;
  y = 1/a1*(Kp*(b1*x+b0*xp)-a0*yp);
  if(y >= 12){
    y = 12;
  }else if(y <= -12){
    y = -12;
  }
  //0.21296791443850267379679144385027*x - 0.23703208556149732620320855614973*xp + yp;
  yp = y;
  xp = x;
  return y;
}
float pidctrl(float e, float Kp, float taui, float taud, float al, float Ts){
  const float b0 = 0.5*Kp*(Ts + 2.0*taud)*(Ts + 2.0*taui);
  const float b1 = 0.5*Kp*(Ts - 2.0*taud)*(Ts + 2.0*taui) + 0.5*Kp*(Ts + 2.0*taud)*(Ts - 2.0*taui);
  const float b2 = 0.5*Kp*(Ts - 2.0*taud)*(Ts - 2.0*taui);
  const float a0 = taui*(Ts + 2.0*al*taud);
  const float a1 = taui*(Ts - 2.0*al*taud) - 1.0*taui*(Ts + 2.0*al*taud);
  const float a2 = -1.0*taui*(Ts - 2.0*al*taud);
  static float x = e, xp = 0, xpp = 0, y = 0, yp = 0, ypp = 0;
  x = e;
  y = 1/a0*(b0*x + b1*xp + b2*xpp - a1*yp - a2*ypp);
  if(y >= 12){
    y = 12;
  }else if(y <= -12){
    y = -12;
  }
  ypp = yp;
  yp = y;
  xpp = xp;
  xp = x;
  return y;
}

float lpfilt(float u, float tau, float Ts){
  float b1 = Ts;
  float b0 = Ts;
  float a1 = Ts+2*tau;
  float a0 = Ts-2*tau;
  static float x = u, xp = u, y = 0, yp = 0;
  x = u;
  y = 1/a1*(b1*x+b0*xp-a0*yp);
  //0.21296791443850267379679144385027*x - 0.23703208556149732620320855614973*xp + yp;
  yp = y;
  xp = x;
  return y;
}
*/
