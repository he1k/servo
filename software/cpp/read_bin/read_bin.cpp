#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <cstring> 
using namespace std;
// CONSTANTS
const char *file_path = "log6.bin";   // "/home/farfar/Documents/servo/software/cpp/read_bin/log1.bin";
const char *file_path_cmp = "log1_serial.bin"; //"/home/farfar/Documents/servo/software/cpp/read_bin/log1.bin";
#define PRINT_CONTENTS 0
#define CHECK_DIFF 1
#define PRINT_DIFF 2
#define PROCESS_TYPE PRINT_CONTENTS
struct logentry_t
{
  uint8_t start;
  int32_t cnt;
  float u;
  uint32_t t;
  float y;
  uint8_t ctrl;
  uint8_t stat;
  uint8_t end;
};
namespace STORAGE
{
  constexpr uint16_t BLOCK_SIZE = 512;
  constexpr uint8_t  N_BLOCKS = 4;
  constexpr uint32_t BFR_SIZE = N_BLOCKS * BLOCK_SIZE;
  constexpr uint8_t  IS_POW_OF_TWO = (BFR_SIZE & (BFR_SIZE - 1)) == 0;
  namespace STATE
  {
    constexpr uint8_t IDLE = 0;    // Idling
    constexpr uint8_t READING = 1; // Reading file
    constexpr uint8_t WRITING = 2; // Writing file
    constexpr uint8_t ERROR = 3;   // Error
  }
  constexpr uint8_t LINE_START     = 0x02; // Start of text
  constexpr uint8_t DATA_SEPERATOR = 0x20; // White space
  constexpr uint8_t LINE_END       = 0x03; // End of text
}
// FUNCTIONS
long get_file_size(const char *filename) {
    FILE *file = fopen(filename, "rb"); // Open the file in binary mode
    if (file == NULL) {
        perror("Failed to open file");
        return -1;
    }

    // Seek to the end of the file
    if (fseek(file, 0, SEEK_END) != 0) {
        perror("Error seeking to end of file");
        fclose(file);
        return -1;
    }

    // Get the current file pointer position
    long size = ftell(file);
    if (size == -1) {
        perror("Error getting file size");
    }

    // Close the file
    fclose(file);
    return size;
}

int32_t find_line_length(uint8_t *bfr)
{
  int32_t line_length = -1;
  int32_t i = 0;
  if(bfr[i] == STORAGE::LINE_START)
  {
    for(i = 0; i < 255; i++)
    {
      if(bfr[i] == STORAGE::LINE_END)
      {
        break;
      }
    }
    line_length = i+1;
  }else
  {
    printf("ERR: Missing LINE_START on first line\n");
  }
  return line_length;
}
uint32_t to_uint32(const uint8_t* array) {
    return (static_cast<uint32_t>(array[0]) << 24) | // Most significant byte
           (static_cast<uint32_t>(array[1]) << 16) |
           (static_cast<uint32_t>(array[2]) << 8)  |
           static_cast<uint32_t>(array[3]);         // Least significant byte
}
int32_t to_int32(const uint8_t* array) {
    uint32_t unsignedResult = (static_cast<uint32_t>(array[0]) << 24) |
                              (static_cast<uint32_t>(array[1]) << 16) |
                              (static_cast<uint32_t>(array[2]) << 8)  |
                              static_cast<uint32_t>(array[3]);

    // Reinterpret as signed
    return static_cast<int32_t>(unsignedResult);
}
void disp_entry(const struct logentry_t* entry) {
    printf("Log Entry:\n");
    printf("  Start: 0x%02X\n", entry->start);      // Print as hex
    printf("  Count: %d\n", entry->cnt);           // Print as signed integer
    printf("  U: %.6f\n", entry->u);               // Print as floating-point with precision
    printf("  Time: %u\n", entry->t);              // Print as unsigned integer
    printf("  Y: %.6f\n", entry->y);               // Print as floating-point with precision
    printf("  Control: 0x%02X\n", entry->ctrl);    // Print as hex
    printf("  Status: 0x%02X\n", entry->stat);     // Print as hex
    printf("  End: 0x%02X\n", entry->end);         // Print as hex
}
/*
{
  const uint8_t start = 0x02;
  //const uint8_t d1 = 0x20;
  int32_t cnt;//float y;
  //const uint8_t d2 = 0x20;
  float u;
  //const uint8_t d3 = 0x20;
  uint32_t t;
  //const uint8_t d4 = 0x20;
  float y;//int32_t cnt;
  //const uint8_t d5 = 0x20;
  uint8_t ctrl;
  //const uint8_t d6 = 0x20;
  uint8_t stat;
  //const uint8_t d7 = 0x20;
  const uint8_t end = 0x03;
}__attribute__((aligned(4)));

*/
#define START_OFFSET 0
#define CNT_OFFSET 1
#define U_OFFSET 5
#define T_OFFSET 9
#define Y_OFFSET 13
#define CTRL_OFFSET 17
#define STAT_OFFSET 18
#define END_OFFSET 19

void line_to_entry(uint8_t *line, int32_t line_len, logentry_t *l)
{
  memcpy(&l->start, &line[START_OFFSET], sizeof(l->start));
  memcpy(&l->cnt,   &line[CNT_OFFSET],   sizeof(l->cnt));
  memcpy(&l->u,     &line[U_OFFSET],     sizeof(l->u));
  memcpy(&l->t,     &line[T_OFFSET],     sizeof(l->t));
  memcpy(&l->y,     &line[Y_OFFSET],     sizeof(l->y));
  memcpy(&l->ctrl,  &line[CTRL_OFFSET],  sizeof(l->ctrl));
  memcpy(&l->stat,  &line[STAT_OFFSET],  sizeof(l->stat));
  memcpy(&l->end,   &line[END_OFFSET],   sizeof(l->end));
}

int main()
{
  FILE *fp;
  FILE *fp_cmp;
  fp = fopen(file_path, "r");
  fp_cmp = fopen(file_path_cmp,"r");
  uint8_t bfr[STORAGE::BFR_SIZE] = {};
  if(!fp)
  {
    printf("ERR: Unable to open file %s\n",file_path);
    return 1;
  }
  if(!fp_cmp)
  {
    printf("ERR: Unable to open file %s\n",file_path_cmp);
    return 1;
  }
  printf("SUCC: Opened files %s and %s\n",file_path, file_path_cmp);
  printf("SUCCESS: Opened files %s and %s\n",file_path, file_path_cmp);
  size_t n = fread(&bfr, 1, STORAGE::BLOCK_SIZE,fp);
  printf("Read:%ld bytes from %s\n",n ,file_path);
  // Find line length:
  uint32_t line_len = 20;//find_line_length(bfr);
  printf("Line length: %d\n",line_len);

  for(uint32_t i = 0; i < 17; i++)
  {


    for(uint32_t j = 0; j < line_len; j++)
    {
      printf("0x%02x ", bfr[i*line_len+j]);
    }
    printf("\n");
  }
  for(uint32_t i = 0; i < 20; i++)
  {
    logentry_t l;
    line_to_entry(bfr+i*line_len,line_len, &l);
    disp_entry(&l);
    //parse_line(bfr+i*line_len,line_len);
  }
  // parse_line(bfr,line_len);
  // parse_line(bfr+line_len,line_len);
  
  // parse_line(bfr,line_len);

  // for(uint32_t i = 0; i < STORAGE::BLOCK_SIZE; i++)
  // {
  //   size_t n = fread(&b, 1, 1, fp); // Read one byte
  //   if(n == 1)
  //   {
  //     printf("0x%02x ", b);
      
  //     if(b == STORAGE::LINE_END)
  //     {
  //       printf("\n");
  //     }
  //   }else
  //   {
  //     if(feof(fp))
  //     {
  //       printf("EOL reaced\n");
  //     }else if(ferror(fp))
  //     {
  //       perror("ERR: Reading file failed\n");
  //     }
  //   }
  // }
  // printf("\n<-\n");
  fclose(fp);
  return 0;
}