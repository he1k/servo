#include <Arduino.h>
#include <SD.h>
#include <SPI.h>
#include <math.h>
#include "constants.h"
#include "motor.h"
#include "storage.h"
const char* write_file = "logs/bin/log9.bin";
Motor mot;
Storage mem;
IntervalTimer tim;
const uint8_t max_n = 20;   // Maximum number of floats
volatile float A[max_n];             // Array of floats
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

int32_t uint32_to_int32(uint32_t value) {
    const uint64_t TWO_POW_32 = 4294967296; // Use 64-bit to represent 2^32
    return (value + INT32_MIN) % TWO_POW_32 - INT32_MIN;
}

namespace STATE
{
  constexpr uint8_t OFF = 0;
  constexpr uint8_t ON = 1;
  constexpr uint8_t TX_LOG = 2;
}


volatile uint8_t tim_flg = 0;
volatile float ref = 0;
volatile float e = 0;
volatile float m = 0;
volatile float u = 1;
volatile uint32_t t = 0;
volatile int32_t cnt_enc = 0;
uint8_t state = STATE::OFF;
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
  /*
  logentry_t lent;
  lent.y = 1.5f;        //0x3FC00000
  lent.u = 3.125f;      //0x40480000
  lent.t = 0xDEADBEEF;  //0xDEADBEEF
  lent.cnt = 0xF8000000;//0xF8000000
  lent.ctrl = 0x0A;     //0x0A
  lent.stat = 0x0B;    //0x0B
  uint8_t *b = (uint8_t *) (&lent);
  Serial.printf("lent size = %lu\n",sizeof(lent));
  Serial.printf("logentry_t size = %lu\n",sizeof(logentry_t));
  Serial.printf("logentry_t size mem = %lu\n",STORAGE::STRUCT_SIZE);
  for (uint32_t i = 0; i < sizeof(lent); i++) 
  {
      Serial.printf("%02x ", b[i]);
  }
  */

  mot.begin();
  mot.write_voltage(0);
  cnt_ms = 0;
  state = STATE::OFF;
  delay(2000);
  if(!mem.begin(STORAGE::CS_SDCARD))
  {
    Serial.println("FAIL SD CARD");
    return;
  }else
  {
    Serial.println("SUCCESS SD CARD");
  }


  if(mem.create_file(write_file))
  {
    Serial.printf("Createdd %s\n",write_file);
  }else
  {
    if(mem.open_file_write(write_file))
    {
      Serial.printf("Successfully opened %s\n",write_file);
    }else
    {
      Serial.printf("Failed to open %s\n",write_file);
      Serial.print("Exists? "); Serial.println(mem.file_exists(write_file));
      return;
  }
  }
  
  //Serial.printf("Size of %s = %lu B\n",write_file, mem.get_file_size(write_file));

  mem.l.y = 1.5f;        //0x3FC00000
  mem.l.u = 3.125f;      //0x40480000
  mem.l.t = 0xDEADBEEF;  //0xDEADBEEF
  mem.l.cnt = 0;//0xF8000000
  mem.l.ctrl = 0x0A;     //0x0A
  mem.l.stat = 0x0B;    //0x0B
  // float A[3] = {0,102.2342529296875f, 200.543212890625f};
  // uint8_t n = 3;
  for(uint32_t i = 0 ; i < 30; i++)
  {
    // uint32_t idx = mem.get_idx();
    // A[0] = (float)i;  
    // mem.queue_line(A,n);
    uint32_t idx = mem.get_idx();
    mem.l.cnt = uint32_to_int32(i);
    mem.l.y   = (float)i;
    mem.l.u   = (float)(i+1);
    mem.l.t   = millis();
    mem.l.stat = 0x42;
    mem.l.ctrl = 0x43;
    uint32_t t1 = micros();
    mem.queue_line_struct();
    t1 = micros() - t1;
    Serial.printf("------------------------------------------------------------\n");
    Serial.printf("dt = %lu.i = %lu, head = %u, tail = %u\n",t1,i, mem.get_head(), mem.get_tail());
    mem.display_buffer_interval(idx,mem.get_idx()-1);
  }
  Serial.printf("WHOLE BUFFER CONTENTS\n ->");
  mem.display_buffer_interval(0,mem.get_idx()-1);
  Serial.printf("<-\n");
  Serial.printf("FIRST BLOCK OF BUFFER:\n->");
  mem.display_buffer_interval(0,511);
  Serial.printf("<-\n");
  Serial.printf("Writing firs block to %s\n",write_file);
  uint32_t t1 = micros();
  uint32_t n_bytes = mem.write_block_to_file();
  t1 = micros() - t1;
  Serial.printf("Wrote %lu bytes to %s dt = %u, closing file %s\n",n_bytes, write_file, t1, write_file);
  mem.close_file();
  //  const uint8_t max_n = 20;
  // volatile float A[max_n]; // Prevent compiler optimizations
  // for (uint8_t i = 0; i < max_n; i++) {
  //     A[i] = random(0, 100) * 0.1f; // Populate with random float values
  // }

  //   for (uint8_t n = 1; n <= max_n; n++) {
  //       unsigned long start_time = micros();
  //       mem.queue_line((float *)A, n);
  //       unsigned long end_time = micros();

  //       unsigned long elapsed_time = end_time - start_time;

  //       Serial.print("n = ");
  //       Serial.print(n);
  //       Serial.print(", Execution time: ");
  //       Serial.print(elapsed_time);
  //       Serial.println(" us");

  //       // Access buffer to prevent optimization
  //       Serial.print("First byte in buffer: ");
  //       Serial.println(mem.bfr[0]);
  //       mem.display_buffer_interval(mem.get_idx())
  //   }


  // mem.card_info();
  // uint32_t t1 = micros();
  // uint32_t t2 = micros();
  // int l = snprintf((char*)mem.bfr, STORAGE::BFR_SIZE, "%lu\t%.5f\t%.5f\t%.5f\t%.5f\t%ld\n",t ,ref, e, m, u, cnt_enc);
  // t2 = micros()-t2;
  // mem.write_block_to_file();//((char*)mem.bfr);
  // t1 = micros() - t1;
  // Serial.print("SD card println time: "); Serial.println(t1);
  // Serial.print("snprintf time : "); Serial.println(t2);
  // Serial.print("l = "); Serial.println(l);
  // Serial.print("bfr(l) = "); Serial.print(mem.bfr[l]);
  // Serial.print("bfr(l-1) = "); Serial.print(mem.bfr[l-1]);
  // mem.close_file();


  tim.begin(tim_isr, TIMER_PERIOD*1e6);
}
void loop()
{
  if(tim_flg)
  {
    tim_flg = 0;
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

