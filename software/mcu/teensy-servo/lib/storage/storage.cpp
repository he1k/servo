#include "storage.h"

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
  bool sd_init = SD.sdfs.begin(SdioConfig(STORAGE::SD_CONFIG));//SD.begin(cs_sd);
  state = sd_init ? STORAGE::STATE::IDLE : STORAGE::STATE::ERROR;
  return sd_init;
}

bool Storage::full()
{
  return ((head == STORAGE::N_BLOCKS - 1) && tail == 0) || ((tail - 1) == head);
}

bool Storage::empty()
{
  return !full() && head == tail;
}
void Storage::idx_inc_wrap()
{
  bfr_idx = (bfr_idx+1) & (STORAGE::BFR_SIZE-1);
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

uint32_t Storage::get_idx()
{
  return bfr_idx;
}

uint8_t Storage::get_head()
{
  return head;
}

uint8_t Storage::get_tail()
{
  return tail;
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
uint8_t Storage::read_clear_error()
{
  uint8_t err = state;
  state = STORAGE::STATE::IDLE;
  return err;
}

// Returns true if a file at path exists, otherwise false
bool Storage::file_exists(const char* path)
{
  return SD.exists(path);
}

// Returns true if a file at path was created, otherwise false
bool Storage::create_file(const char* path)
{
  // Check if other operation or error is present
  if(state != STORAGE::STATE::IDLE)
  {
    Serial.println("ERR: Not in idle mode");
    state = STORAGE::STATE::ERROR;
    return false;
  }
  // Check if file already exists
  if(file_exists(path))
  {
    Serial.println("ERR: File already exists");
    state = STORAGE::STATE::ERROR;
    return false;
  }
  // Create the file
  curr_file = SD.open(path, FILE_WRITE);
  if(curr_file)
  {
    curr_file.close();
    return true;
  }else
  {
    Serial.println("ERR: Couldn't create file");
    state = STORAGE::STATE::ERROR;
    return false;
  }
}

// Returns true if file at path was opened successfully for reading, otherwise false
bool Storage::open_file_read(const char* path)
{
  // Check if other operation or error is present
  if(state != STORAGE::STATE::IDLE)
  {
    Serial.println("ERR: Not in idle mode");
    state = STORAGE::STATE::ERROR;
    return false;
  }
  // Check if file exists
  if(!file_exists(path))
  {
    Serial.println("ERR: File does not exist");
    state = STORAGE::STATE::ERROR;
    return false;
  }
  curr_file = SD.open(path, FILE_READ);
  if(!curr_file)
  {
    Serial.println("ERR: Opening file for reading failed");
    state = STORAGE::STATE::ERROR;
    return false;
  }
  state = STORAGE::STATE::READING;
  return true;
}
bool Storage::open_file_write(const char* path)
{
  // Check if other operation or error is present
  if(state != STORAGE::STATE::IDLE)
  {
    Serial.println("ERR: Not in idle mode");
    state = STORAGE::STATE::ERROR;
    return false;
  }
  // Check if file exists
  if(!file_exists(path))
  {
    Serial.println("ERR: File does not exist");
    state = STORAGE::STATE::ERROR;
    return false;
  }
  curr_file = SD.open(path, FILE_WRITE);
  if(!curr_file)
  {
    Serial.println("ERR: Opening file for writing failed");
    state = STORAGE::STATE::ERROR;
    return false;
  }
  state = STORAGE::STATE::WRITING;
  return true;
}
bool Storage::write_line_to_file(const char* line)
{
  if(state == STORAGE::STATE::WRITING)
  {
    curr_file.print(line);
    return true;
  }else
  {
    Serial.println("ERR: No file opened for writing");
    state = STORAGE::STATE::ERROR;
    return false;
  }
}

__UINT_LEAST32_TYPE__ Storage::write_block_to_file()
{
  if(state == STORAGE::STATE::WRITING)
  {
    if(!empty())
    {
      uint32_t n_bytes = curr_file.write(bfr + tail * STORAGE::BLOCK_SIZE,STORAGE::BLOCK_SIZE);
      if(++tail == STORAGE::N_BLOCKS) // Adjust tail
      {
        tail = 0;
      }
      return n_bytes;
    }else
    {
      Serial.println("ERR: Empty ring buffer (no full block)");
      state = STORAGE::STATE::ERROR;
      return 0;
    }
  }else
  {
    Serial.println("ERR: No file open");
    state = STORAGE::STATE::ERROR;
    return false;
  }
}

void Storage::close_file()
{
  curr_file.close();
}

void Storage::list_all_files(const char* path, int depth) {
  File dir = SD.open(path);
  if (!dir || !dir.isDirectory())
  {
    Serial.println("Not a directory or does not exist.");
    return;
  }
  File file;
  while ((file = dir.openNextFile()))
  {
    for (int i = 0; i < depth; i++)
    {
      Serial.print("  "); // Indent for subdirectories
    }

    if (file.isDirectory())
    {
      Serial.print("[DIR] ");
      Serial.println(file.name());
      // Recursive call to list files in the subdirectory
      list_all_files(file.name(), depth + 1);
    }
    else
    {
      Serial.print("[FILE] ");
      Serial.print(file.name());
      Serial.print("  SIZE: ");
      Serial.println(file.size());
    }
    file.close();
  }
}


// 
void Storage::card_info()
{
  Sd2Card card;
  SdVolume volume;
  SdFile root;
   // we'll use the initialization code from the utility libraries
  // since we're just testing if the card is working!
  if (!card.init(SPI_HALF_SPEED, STORAGE::CS_SDCARD)) {
    Serial.println("initialization failed. Things to check:");
    Serial.println("* is a card inserted?");
    Serial.println("* is your wiring correct?");
    Serial.println("* did you change the chipSelect pin to match your shield or module?");
    return;
  } else {
   Serial.println("Wiring is correct and a card is present.");
  }

  // print the type of card
  Serial.print("\nCard type: ");
  switch(card.type()) {
    case SD_CARD_TYPE_SD1:
      Serial.println("SD1");
      break;
    case SD_CARD_TYPE_SD2:
      Serial.println("SD2");
      break;
    case SD_CARD_TYPE_SDHC:
      Serial.println("SDHC");
      break;
    default:
      Serial.println("Unknown");
  }

  // Now we will try to open the 'volume'/'partition' - it should be FAT16 or FAT32
  if (!volume.init(card)) {
    Serial.println("Could not find FAT16/FAT32 partition.\nMake sure you've formatted the card");
    return;
  }


  // print the type and size of the first FAT-type volume
  uint32_t volumesize;
  Serial.print("\nVolume type is FAT");
  Serial.println(volume.fatType(), DEC);
  Serial.println();
  
  volumesize = volume.blocksPerCluster();    // clusters are collections of blocks
  volumesize *= volume.clusterCount();       // we'll have a lot of clusters
  if (volumesize < 8388608ul) {
    Serial.print("Volume size (bytes): ");
    Serial.println(volumesize * 512);        // SD card blocks are always 512 bytes
  }
  Serial.print("Volume size (Kbytes): ");
  volumesize /= 2;
  Serial.println(volumesize);
  Serial.print("Volume size (Mbytes): ");
  volumesize /= 1024;
  Serial.println(volumesize);
}