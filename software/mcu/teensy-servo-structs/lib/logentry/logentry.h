#ifndef LOG_ENTRY_H
#define LOG_ENTRY_H
#include <Arduino.h>

class LogEntry
{
  public:
    LogEntry();
    float y;
    float u;
    uint32_t t;
    int32_t cnt;
    uint8_t ctrl;
    
};


#endif