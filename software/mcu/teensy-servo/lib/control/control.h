#ifndef CONTROL_H
#define CONTROL_H
#include <Arduino.h>
#define OUT_OF_BOUNDS 9999.1337
#define TS_DEF 0.001
#define KP_DEF 1.75
#define TAU_I_DEF 0.01
#define TAU_D_DEF 0.1
#define ALPHA_DEF 0.4
#define E_DEF 1.0
#define LIM_DEF 1e9
class PID
{
private:
  float _Ts;
  float _Kp;
  float _tau_i;
  float _tau_d;
  float _alpha;
  float _lim;
  bool _en_lim;
  float _a[3];
  float _b[3];
  float _u[3];
  float _e[3];

public:
  PID();
  void begin(float Ts, float Kp, float tau_i, float tau_d, float alpha, float lim, bool en_lim);
  float update(float r, float m);
  void reset();
  double getTs();
  double getKp();
  double getTaui();
  double getTaud();
  double getA(int idx);
  double getB(int idx);
  void test(float Ts, float Kp, float tau_i, float tau_d, float alpha, float lim, bool en_lim, float e);
  void test(float e);
  void print();
};


#endif