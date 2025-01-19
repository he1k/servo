#ifndef MY_STRUCTS_H
#define MY_STRUCTS_H
#include <Arduino.>
struct logentry_t
{
  uint32_t t;
  float u;
  float y;
  int32_t cnt;
  uint8_t ctrl;
};

#neid