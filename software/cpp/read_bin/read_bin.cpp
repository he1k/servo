#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
using namespace std;
// CONSTANTS
const char *file_path = "log4.bin";   // "/home/farfar/Documents/servo/software/cpp/read_bin/log1.bin";
const char *file_path_cmp = "log1_serial.bin"; //"/home/farfar/Documents/servo/software/cpp/read_bin/log1.bin";
#define PRINT_CONTENTS 0
#define CHECK_DIFF 1
#define PRINT_DIFF 2
#define PROCESS_TYPE PRINT_CONTENTS
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

uint8_t parse_line(uint8_t *line, int32_t line_len)
{
  if(line[0] != STORAGE::LINE_START || line[line_len - 1] != STORAGE::LINE_END)
  {
    printf("ERR: Missing LINE_START or LINE_END\n");
    return 0;
  }
  for(uint32_t i = 0; i < line_len; i++)
  {
    printf("0x%02x ", line[i]);
  }
  printf("\n");
  uint8_t n = (18-3)/5;
  printf("n = %d\n",n);
  uint8_t state = 0;
  uint8_t b[n][4];
  float A[n];
  float *f;
  uint32_t idx = 0;
  for(uint8_t i = 0; i < n; i++)
  {
    for(uint8_t j = 0; j < 4; j++)
    {
      b[i][j] = line[2+i*4+i+j];
    }
    f = (float*) b[i];
    printf("%f ",*f);
  }
  

  //A[0] = 
  // for(uint32_t i = 0; i <= line_len; i++)
  // {
  //   if(state == 0)
  //   {
  //     if(line[i] == STORAGE::DATA_SEPERATOR)
  //     {
  //       state = 1;
  //     }
  //   }else if(state == 1)
  //   {
  //     if(line[i] == STORAGE::DATA_SEPERATOR)
  //     {
  //       // for(uint32_t j = 0; j < idx; j++)
  //       // {
  //       //   printf("b: 0x%02x ",b[j]);
  //       // }
  //       printf("\n");
  //       f = (float*) b;
  //       printf("%f ",*f);
  //       state = 0;
  //       idx = 0;
  //     }else
  //     {
  //       b[idx] = line[i];  
  //       idx++;
  //     }
  //   }
  // }
  return 1;
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
  int32_t line_len = 27;//find_line_length(bfr);
  printf("Line length: %d\n",line_len);

  
  for(uint32_t i = 0; i < 17; i++)
  {
    for(uint32_t j = 0; j < line_len; j++)
    {
      printf("0x%02x ", bfr[i*line_len+j]);
    }
    uint8_t b[4];
    uint32_t start_idx = 2;
    for(uint32_t j = 0; j < 4; j++)
    {
      b[j] = bfr[i*line_len+start_idx];
    }
    int32_t cnt = (b[0] << 24) |
                  (b[1] << 16) |
                  (b[2] << 8)  |
                  b[3];
    printf("cnt = %d\n",b[0]);
    printf("\n");
  }
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