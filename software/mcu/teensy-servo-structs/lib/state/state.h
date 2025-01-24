#ifndef STATE_H
#define STATE_H
#include <Arduino.h>
namespace STATE
{
  namespace STATE
  {
    constexpr uint8_t OFF    = 0;
    constexpr uint8_t ON     = 1;
    constexpr uint8_t TX_LOG = 2;
  }
  namespace CMD
  {
    constexpr uint8_t NONE      = 0x00;
    constexpr uint8_t BEGIN     = (uint8_t) 'B';
    constexpr uint8_t END       = (uint8_t) 'S';
    constexpr uint8_t TX_LOG    = (uint8_t) 'L';
    constexpr uint8_t LINE_FEED = (uint8_t) '\n';
  }

}
class State
{
  public:
    uint8_t state;
    uint32_t cnt;
    State();
    void begin();
    void update(uint8_t cmd);
};



#endif