#include "state.h"

State::State()
{
  
}

void State::begin()
{
  state = STATE::STATE::OFF;
  cnt = 0;
}

void State::update(uint8_t cmd)
{
  switch(state)
  {
    case STATE::STATE::OFF:
      cnt = 0;
      if(cmd == STATE::CMD::BEGIN)
      {
        state = STATE::STATE::ON;
      }  
      break;
    case STATE::STATE::ON:
      cnt++;
      if(cmd == STATE::CMD::END)
      {
        state = STATE::STATE::OFF;
      }
      break;
    case STATE::STATE::TX_LOG:
      if(cmd == STATE::CMD::END)
      {
        state = STATE::STATE::OFF;
      }
      break;
  }
}