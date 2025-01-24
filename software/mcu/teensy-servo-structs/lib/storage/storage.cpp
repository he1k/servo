#include "storage.h"

// --------------------------------------------------------------------- //
// INIT FUNCTIONS
// Empty constructor
Storage::Storage()
{

}
// Initialize the Storage object.
// Returns true if connection to SD card was estabilshed, otherwise false
bool Storage::begin(uint8_t cs)
{
  cs_sd = cs;
  for(uint32_t i = 0; i < STORAGE::BFR_SIZE; i++)
  {
    bfr[i] = 0;
  }
  bfr_idx = 0;
  head = 0;
  tail = 0;
  curr_file = nullptr;
  f_write = nullptr;
  f_read = nullptr;
  state = STORAGE::STATE::IDLE;
  err = STORAGE::ERR::NONE;
  bool sd_init = SD.sdfs.begin(SdioConfig(FIFO_SDIO));//SD.begin(cs_sd);
  return sd_init;
}

// Returns the current value of the error field
// clears the field at the same time
uint8_t Storage::get_error()
{
  uint8_t tmp = err;
  err = STORAGE::ERR::NONE;
  return tmp;
}
// --------------------------------------------------------------------- //
// BUFFER FUNCTIONS
// Returns if the circular buffer is full

bool Storage::full()
{
  return ((head == STORAGE::N_BLOCKS - 1) && tail == 0) || ( (tail > head ) && (tail - head == 1));
  // if((head == STORAGE::N_BLOCKS - 1) && tail == 0)
  // {
  //   return true;
  // }
  // if(tail > head)
  // {
  //   if(tail - head == 1)
  //   {
  //     return true;
  //   }
  // }

  // return false;
}

// Returns if the circular buffer is empty
bool Storage::empty()
{
  return head == tail;
}

// Returns head of the circular buffer
uint8_t Storage::get_head()
{
  return head;
}

// Returns tail of the circular buffer
uint8_t Storage::get_tail()
{
  return tail;
}

// Increments the absolute index of the byte buffer (wraps to 0)
void Storage::idx_inc_wrap()
{
  bfr_idx = (bfr_idx+1) & (STORAGE::BFR_SIZE-1);
}

// Returns absolute index for the byte buffer
uint32_t Storage::get_idx()
{
  return bfr_idx;
}

// Converts current log struct to bytes and inserts them into the byte buffer
bool Storage::queue_line_struct()
{
  uint32_t block_offset = bfr_idx % STORAGE::BLOCK_SIZE;
  uint8_t overlap = (block_offset + STORAGE::STRUCT_SIZE) >= STORAGE::BLOCK_SIZE - 1;
  if(overlap and full())
  {
    Serial.println("ERR: Overlap and full");
    return false;
  }
  uint8_t *b = (uint8_t *) &l;
  for(uint32_t i = 0; i < STORAGE::STRUCT_SIZE; i++)
  {
    bfr[bfr_idx] = b[i];
    idx_inc_wrap();
  }
  // Since the byte pointer has been wrapped each time, we can execute the same code
  // regardless of overlap and which type. Lastly we adjust head pointer if overlap was detected
  if(overlap)
  {
    if(++head == STORAGE::N_BLOCKS)
    {
      head = 0;
    }
  }
  return true;
  
}

bool Storage::queue_line(float *A, uint8_t n)
{
  // Check if data fits into current block 
  // START_BYTE + DEL_BYTE + 4*n(0) + DEL_BYTE + END_BYTE
  uint32_t n_bytes = 5 * n + 3;
  uint32_t block_offset = bfr_idx % STORAGE::BLOCK_SIZE;
  uint8_t overlap = (block_offset + n_bytes) >= STORAGE::BLOCK_SIZE - 1;
  // uint32_t bfr_idx_cur = head * STORAGE::BLOCK_SIZE + block_offset;
  if(overlap and full())
  {
    Serial.println("ERR: Overlap and full");
    return false;
  }
  bfr[bfr_idx] = STORAGE::LINE_START;       // Insert start byte
  idx_inc_wrap();                           // Increment byte pointer and wrap it such that it does not go out of bounds
  bfr[bfr_idx] = STORAGE::DATA_SEPERATOR;   // Insert seperator byte
  idx_inc_wrap();
  for(uint8_t i = 0; i < n; i++)              // Loop through the n floats
  {
    uint8_t *f_b = (uint8_t *)&A[i];          // Convert float to bytes
    bfr[bfr_idx] = f_b[0];                  // Store A[i] using little endian 
    idx_inc_wrap();
    bfr[bfr_idx] = f_b[1];
    idx_inc_wrap();
    bfr[bfr_idx] = f_b[2];
    idx_inc_wrap();
    bfr[bfr_idx] = f_b[3];
    idx_inc_wrap();
    bfr[bfr_idx] = STORAGE::DATA_SEPERATOR; // Insert seperator byte
    idx_inc_wrap();
  }
  bfr[bfr_idx] = STORAGE::LINE_END;       // Insert end byte
  idx_inc_wrap();
  // Since the byte pointer has been wrapped each time, we can execute the same code
  // regardless of overlap and which type. Lastly we adjust head pointer if overlap was detected
  if(overlap)
  {
    if(++head == STORAGE::N_BLOCKS)
    {
      head = 0;
    }
  }
  return true;

  // Calculate current write index and next write index (based on input)
  // uint32_t bfr_idx_cur = head * STORAGE::BLOCK_SIZE + block_offset;
  // uint32_t bfr_idx_next = bfr_idx_cur + 5 * n + 3;


  // We cannot insert the data in the case where:


  // For each n we need 4 bytes of data
  // Each line must start out with a line start byte
  // Each line must end with a line end byte
  // Between both data bytes (groups of 4) and SOL and EOL bytes there must be a seperator (white space)
  // therefore n+1 seperator bytes are needed.
  // The total amount of bytes for a line is:
  //             line_start + data + seperators + line_end
  //             (1) + (4*n) + (n+1) + (1) = 5*n+3

  // uint32_t bfr_idx_next = bfr_idx + 5 * n + 3;
  // if(bfr_idx_next > STORAGE::BFR_SIZE) // Too much data in buffer
  // {
  //   return false;
  // }else // We can insert line into buffer
  // {
  //   bfr[bfr_idx++] = STORAGE::LINE_START;       // Insert start byte
  //   bfr[bfr_idx++] = STORAGE::DATA_SEPERATOR;   // Insert seperator byte
  //   for(uint8_t i = 0; i < n; i++)              // Loop through the n floats
  //   {
  //     uint8_t *f_b = (uint8_t *)&A[i];          // Convert float to bytes
  //     bfr[bfr_idx++] = f_b[0];                  // Store A[i] using little endian 
  //     bfr[bfr_idx++] = f_b[1];
  //     bfr[bfr_idx++] = f_b[2];
  //     bfr[bfr_idx++] = f_b[3];
  //     bfr[bfr_idx++] = STORAGE::DATA_SEPERATOR; // Insert seperator byte
  //   }
  //   bfr[bfr_idx++] = STORAGE::LINE_END;       // Insert end byte
  //   return true;
  // }
}

void Storage::display_buffer_interval(uint32_t idx_start, uint32_t idx_end)
{
  if(idx_start < 0 || idx_end >= STORAGE::BFR_SIZE)
  {
    Serial.println("ERR: Indicies out of bounds");
    return;
  }else
  {
    Serial.printf("Buffer contents from %lu to %lu\n-\n",idx_start, idx_end);
    for (uint32_t i = idx_start; i <= idx_end; i++) {
      Serial.printf("0x%02x ", bfr[i]);
      if(bfr[i] == STORAGE::LINE_END)
      {
        Serial.printf("\n");
      }
    }
    Serial.printf("-\n");
  }
}

// --------------------------------------------------------------------- //
// FILE FUNCTIONS

// Delete file at path
void Storage::delete_file(const char* path)
{
  SD.remove(path);
}

// Returns true if a file at path exists, otherwise false
bool Storage::file_exists(const char* path)
{
  return SD.exists(path);
}

void Storage::close_file(uint8_t write)
{
  if(write)
  {
    f_write.close();
  }else{
    f_read.close();
  }
}


// Creates a file at the given path
// If the file already exists, then it is deleted first
// such as to ensure an empty file is created
bool Storage::create_empty_file(const char* path, uint64_t f_size)
{
  if(file_exists(path))
  {
    Serial.println("File exist");
    delete_file(path);
  }
  FsFile f = SD.sdfs.open(path, FILE_WRITE);
  if(f)
  {
    f.preAllocate(f_size);
    f.close();
    return true;
  }else
  {
    err = STORAGE::ERR::FILE_NOT_CREATED;
    return false;
  }
}

// Returns true if file at path was opened successfully for reading, otherwise false
bool Storage::open_file_read(const char* path)
{
  // Check if file exists
  if(!file_exists(path))
  {
    err = STORAGE::ERR::FILE_NOT_EXIST;
    return false;
  }
  // Open file for reading
  f_read = SD.open(path, FILE_READ);
  if(!f_read) // Failed to open file
  {
    err = STORAGE::ERR::READ_FILE_NOT_OPENED;
    return false;
  }
  return true;
}
bool Storage::open_file_write(const char* path)
{
  // Check if file exists
  if(!file_exists(path))
  {
    err = STORAGE::ERR::FILE_NOT_EXIST;
    return false;
  }
  // Open file for writing
  f_write = SD.open(path, FILE_WRITE);
  if(!f_write) // Failed to open file
  {
    err = STORAGE::ERR::WRITE_FILE_NOT_OPENED;
    return false;
  }
  return true;
}
bool Storage::write_line_to_file(const char* line)
{
  if(f_write)
  {
    f_write.print(line);
    return true;
  }else
  {
    err = STORAGE::ERR::WRITE_POINTER_NOT_OPEN;
    return false;
  }
}

uint32_t Storage::write_block_to_file()
{
  uint32_t n_bytes = -1;
  if(f_write)
  {
    if(!empty())
    {
      n_bytes = f_write.write(bfr + tail * STORAGE::BLOCK_SIZE,STORAGE::BLOCK_SIZE);
      if(++tail == STORAGE::N_BLOCKS) // Adjust tail
      {
        tail = 0;
      }
    }
  }else
  {
    err = STORAGE::ERR::WRITE_POINTER_NOT_OPEN;
  }
  return n_bytes;
}

int64_t Storage::get_file_size(const char* path)
{
  int64_t size = -1;
  // Check if file exists
  if(!file_exists(path))
  {
    err = STORAGE::ERR::FILE_NOT_EXIST;
  }else
  {
    File f = SD.open(path, FILE_READ);
    if(f)
    {
      size = f.size();
      f.close();
    }else
    {
      err = STORAGE::ERR::FILE_SIZE_NOT_OPENED;
    }
  }
  return size;
}

// void Storage::list_all_files(const char* path, int depth) {
//   File dir = SD.open(path);
//   if (!dir || !dir.isDirectory())
//   {
//     Serial.println("Not a directory or does not exist.");
//     return;
//   }
//   File file;
//   while ((file = dir.openNextFile()))
//   {
//     for (int i = 0; i < depth; i++)
//     {
//       Serial.print("  "); // Indent for subdirectories
//     }

//     if (file.isDirectory())
//     {
//       Serial.print("[DIR] ");
//       Serial.println(file.name());
//       // Recursive call to list files in the subdirectory
//       list_all_files(file.name(), depth + 1);
//     }
//     else
//     {
//       Serial.print("[FILE] ");
//       Serial.print(file.name());
//       Serial.print("  SIZE: ");
//       Serial.println(file.size());
//     }
//     file.close();
//   }
// }