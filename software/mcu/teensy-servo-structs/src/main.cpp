#include <Arduino.h>
#include <SD.h>
#include <SPI.h>
#include <math.h>
#include "constants.h"
#include "motor.h"
#include "storage.h"
#include "state.h"

const char* write_file = "/write/bin/log1.bin";
const uint64_t f_prealloc = 40*1024*1024; // 40 MB preallocate
const char* read_file = "/read/parameters/gains.txt";

// Motor object
Motor mot; 
// Memory object
Storage mem;
// State object
State sta;
// Timer object
IntervalTimer tim;

#define SINE_FREQ 0.25f
#define KP 2.5
#define DIGITS 5
#define TIMER_PERIOD 0.001
#define PRINT_PERIOD 0.4
#define CNT_MS_MAX (uint32_t) (PRINT_PERIOD/TIMER_PERIOD)
#define T_SAMPLE 10.0
#define N_SAMPLES (uint32_t) (T_SAMPLE/TIMER_PERIOD)
#define SQUARE_AMPLITUDE 2*M_PI
#define SQUARE_PERIOD 0.5
#define SQUARE_CNT_MAX (uint32_t) (SQUARE_PERIOD/TIMER_PERIOD)

// namespace STATE
// {
//   constexpr uint8_t IDLE   = 0;
//   constexpr uint8_t OFF    = 1;
//   constexpr uint8_t ON     = 2;
//   constexpr uint8_t TX_LOG = 3;
// }


volatile uint8_t tim_flg = 0;
volatile float ref = 0;
volatile float e = 0;
volatile float m = 0;
volatile float u = 1;
volatile uint32_t t = 0;
volatile int32_t cnt_enc = 0;
//uint8_t state = STATE::OFF;
uint32_t cnt_ms = 0;
File myFile;



void tim_isr()
{
  tim_flg = 1;
}
void setup()
{
  Serial.begin(COMM::BAUD_RATE);
  while(!Serial){};
  mot.begin();
  sta.begin();
  cnt_ms = 0;
  //state = STATE::OFF;
  delay(2000);
  if(!mem.begin(STORAGE::CS_SDCARD))
  {
    Serial.println("FAIL SD CARD");
    return;
  }else
  {
    Serial.println("SUCCESS SD CARD");
  }
  delay(2000);
  // int64_t f_size = mem.get_file_size(write_file);
  // uint8_t e = mem.get_error();
  // Serial.print("Size of file: "); Serial.print(f_size); Serial.print("\t e= "); Serial.println(e);
  //Serial.printf("Size of %s = %ld B, e = %u\n",write_file, f_size, e);
  bool o = mem.create_empty_file(write_file,f_prealloc);
  Serial.print("create_empty_file = "); Serial.print(o); Serial.print("\t e = "); Serial.println(mem.get_error());
  o = mem.open_file_write(write_file);
  Serial.print("open_file_write = "); Serial.print(o); Serial.print("\t e = "); Serial.println(mem.get_error());
  int64_t f_size = mem.get_file_size(write_file);
  uint8_t er = mem.get_error();
  Serial.print("Size of file: "); Serial.print(f_size); Serial.print("\t e= "); Serial.println(e);
  //Serial.printf("Size of %s = %d B, e = %u\n",write_file, f_size, e);
  if(!o)
  {
    while(1){};
  }
  tim.begin(tim_isr, TIMER_PERIOD*1e6);
}

void loop()
{
  if(tim_flg)
  {
    tim_flg = 0;
    uint8_t cmd = STATE::CMD::NONE;
    if(Serial.available() > 1)
    {
      uint8_t b1 = Serial.read();
      uint8_t b2 = Serial.read();
      if(b2 == STATE::CMD::LINE_FEED)
      {
        cmd = b1;
      }  
    }
    if(sta.cnt == N_SAMPLES)
    {
      cmd = STATE::CMD::END;
      mem.close_file(1);
    }
    sta.update(cmd);
    switch(sta.state)
    {
      case STATE::STATE::OFF:
        mot.clear();  
        ref = SQUARE_AMPLITUDE;
        break;
      case STATE::STATE::ON:
        //ref = 2*M_PI;//2*M_PI*sin(2.0f*M_PI*SINE_FREQ*cnt_ms*1e-3);
        if(sta.cnt < SQUARE_CNT_MAX)
        {
          ref = 0;
        }else if(sta.cnt == SQUARE_CNT_MAX)
        {
          ref = SQUARE_AMPLITUDE;
        }else if(sta.cnt % SQUARE_CNT_MAX == 0)
        {
          ref = -1*ref;
        }
        m = mot.read_position();
        e = ref-m;
        u = KP*e;
        mot.write_voltage(u);
        mem.l.t = micros();
        mem.l.y = m;
        mem.l.u = mot.read_voltage();
        mem.l.r = ref;
        mem.l.e = e;
        mem.l.ctrl = mem.get_head();   
        mem.l.state = mem.get_tail();    
        mem.queue_line_struct();
        if(!mem.empty())
        {
          //uint32_t t1 = micros();
          // uint32_t n_bytes = 
          mem.write_block_to_file();
          //t1 = micros() - t1;
          //Serial.print("n_bytes = "); Serial.print(n_bytes); Serial.print("\t dt ="); Serial.print(t1); Serial.print("\t");
          //Serial.print("head = "); Serial.print(mem.get_head()); Serial.print("\t tail = "); Serial.println(mem.get_tail());
        }
        break;
      case STATE::STATE::TX_LOG:
        break;
  }
    /*
    if(state == STATE::ON)
    {
      //ref = 2*M_PI;//2*M_PI*sin(2.0f*M_PI*SINE_FREQ*cnt_ms*1e-3);
      if(cnt_ms < SQUARE_CNT_MAX)
      {
        ref = 0;
      }else if(cnt_ms == SQUARE_CNT_MAX)
      {
        ref = SQUARE_AMPLITUDE;
      }else if(cnt_ms % SQUARE_CNT_MAX == 0)
      {
        ref = -1*ref;
      }
      m = mot.read_position();
      e = ref-m;
      u = KP*e;
      mot.write_voltage(u);
      if(cnt_ms < N_SAMPLES)
      {
        data_log[cnt_ms].t = micros();
        data_log[cnt_ms].r = ref;
        data_log[cnt_ms].e = e;
        data_log[cnt_ms].m = m;
        data_log[cnt_ms].u = mot.read_voltage();
        data_log[cnt_ms].cnt = mot.read_count();
        cnt_ms++;
      }else
      {
        state = STATE::OFF;
        //Serial.println("Test done");
      }
    }else if(state == STATE::OFF)
    {
      cnt_ms = 0;
      mot.clear();
      ref = SQUARE_AMPLITUDE;
      if(Serial.available() > 1)
      {
        uint8_t b1 = Serial.read();
        uint8_t b2 = Serial.read();
        if(b2 != (uint8_t) '\n')
        {
          //Serial.println("Missing LF");
        }else
        {
          if(b1 == (uint8_t) 'S')
          {
            //Serial.println("Starting test");
            state = STATE::ON;
          }else if(b1 == (uint8_t) 'L')
          {
            //Serial.println("Dumping log");
            state = STATE::TX_LOG;
          }
        }
      }
    }else if(state == STATE::TX_LOG)
    {
      if(cnt_ms < N_SAMPLES)
      {
        Serial.print(data_log[cnt_ms].t); Serial.print("\t");
        Serial.print(data_log[cnt_ms].r ,DIGITS); Serial.print("\t");
        Serial.print(data_log[cnt_ms].e ,DIGITS); Serial.print("\t");
        Serial.print(data_log[cnt_ms].m ,DIGITS); Serial.print("\t");
        Serial.print(data_log[cnt_ms].u ,DIGITS); Serial.print("\t");
        Serial.print(data_log[cnt_ms].cnt);
        Serial.print("\n");
        cnt_ms++;
      }else
      {
        state = STATE::OFF;
      }
    }
    */
  }
}


// void loop()
// {
//   if(tim_flg)
//   {
//     tim_flg = 0;
//     /*
//     if(state == STATE::ON)
//     {
//       //ref = 2*M_PI;//2*M_PI*sin(2.0f*M_PI*SINE_FREQ*cnt_ms*1e-3);
//       if(cnt_ms < SQUARE_CNT_MAX)
//       {
//         ref = 0;
//       }else if(cnt_ms == SQUARE_CNT_MAX)
//       {
//         ref = SQUARE_AMPLITUDE;
//       }else if(cnt_ms % SQUARE_CNT_MAX == 0)
//       {
//         ref = -1*ref;
//       }
//       m = mot.read_position();
//       e = ref-m;
//       u = KP*e;
//       mot.write_voltage(u);
//       if(cnt_ms < N_SAMPLES)
//       {
//         data_log[cnt_ms].t = micros();
//         data_log[cnt_ms].r = ref;
//         data_log[cnt_ms].e = e;
//         data_log[cnt_ms].m = m;
//         data_log[cnt_ms].u = mot.read_voltage();
//         data_log[cnt_ms].cnt = mot.read_count();
//         cnt_ms++;
//       }else
//       {
//         state = STATE::OFF;
//         //Serial.println("Test done");
//       }
//     }else if(state == STATE::OFF)
//     {
//       cnt_ms = 0;
//       mot.clear();
//       ref = SQUARE_AMPLITUDE;
//       if(Serial.available() > 1)
//       {
//         uint8_t b1 = Serial.read();
//         uint8_t b2 = Serial.read();
//         if(b2 != (uint8_t) '\n')
//         {
//           //Serial.println("Missing LF");
//         }else
//         {
//           if(b1 == (uint8_t) 'S')
//           {
//             //Serial.println("Starting test");
//             state = STATE::ON;
//           }else if(b1 == (uint8_t) 'L')
//           {
//             //Serial.println("Dumping log");
//             state = STATE::TX_LOG;
//           }
//         }
//       }
//     }else if(state == STATE::TX_LOG)
//     {
//       if(cnt_ms < N_SAMPLES)
//       {
//         Serial.print(data_log[cnt_ms].t); Serial.print("\t");
//         Serial.print(data_log[cnt_ms].r ,DIGITS); Serial.print("\t");
//         Serial.print(data_log[cnt_ms].e ,DIGITS); Serial.print("\t");
//         Serial.print(data_log[cnt_ms].m ,DIGITS); Serial.print("\t");
//         Serial.print(data_log[cnt_ms].u ,DIGITS); Serial.print("\t");
//         Serial.print(data_log[cnt_ms].cnt);
//         Serial.print("\n");
//         cnt_ms++;
//       }else
//       {
//         state = STATE::OFF;
//       }
//     }
//     */
//   }
// }

