#ifndef STORAGE_H
#define STORAGE_H
#include <Arduino.h>
#include <SD.h>
#include <SPI.h>
namespace STORAGE
{
  constexpr uint8_t CS_SDCARD = BUILTIN_SDCARD;
  constexpr uint8_t SD_CONFIG = FIFO_SDIO;
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

class Storage
{
  private:
    uint8_t cs_sd; // CS for SD card reader
    uint32_t bfr_idx;
    uint8_t state;
    uint8_t head;       // Circular buffer head pointer
    uint8_t tail;       // Circular buffer tail pointer
    File curr_file;     // Current file
    void idx_inc_wrap();
  public:
    uint8_t bfr[STORAGE::BFR_SIZE];
    /**
     * Empty constructor
     */
    Storage();
    /**
     * Initialize the object. 
     * \param cs is the chip select pin for the SD card
     * \returns result of communication establishment with SD card
     */
    bool begin(uint8_t cs);
    // Circular buffer functions
    bool empty();
    bool full();




    /**
     * Prints the whole file tree
     * \param path is the path to the file
     * \param depth used for formatting subdirectories
     * \returns void
     */
    void list_all_files(const char* path, int depth = 0);
    /**
     * Show card information
     * \returns void
     */
    void card_info();

    /**
     * Inserts a line of data into the buffer
     * \param A floating point numbers for the current line
     * \param n number of elements in A 
     * \returns true if the buffer does not overflow, otherwise false
     */
    bool queue_line(float *A, uint8_t n);

     /**
     * Gets current bfr idx;
     * \returns current buffer index
     */
    uint32_t get_idx();
    /**
     * Gets current buffer head
     * \returns current buffer head
     */
    uint8_t get_head();
    /**
     * Gets current buffer tail
     * \returns current buffer tail
     */
    uint8_t get_tail();
    /**
     * Display's the contents of the buffer in an interval
     * \param idx_start floating point numbers for the current line
     * \param idx_end number of elements in A 
     * \returns void
     */
    void display_buffer_interval(uint32_t idx_start, uint32_t idx_end);

    /**
     * Check if file with path exists
     * \param path is the path to the file
     * \returns if the file exists on the SD card.
     */
    bool file_exists(const char* path);
    /**
     * Create a new file with path
     * \param path is the path to the file
     * \returns if the file was created
     * \note can return false due to multiple reasons
     */
    bool create_file(const char* path);
    /**
     * Open file at path for reading only
     * \param path is the path to the file
     * \returns if the file was opened for reading successfully 
     * \note can return false due to multiple reasons
     */
    bool open_file_read(const char* path);
    /**
     * Open file at path for writing only
     * \param path is the path to the file
     * \returns if the file was opened for writing successfully 
     * \note can return false due to multiple reasons
     */
    bool open_file_write(const char* path);
    /**
     * Writes a line (string) 
     * \param line is the path to the file
     * \returns if the line was written to the file
     * \note there is no need path provided, as this
     *       function assumes there is already a file
     *       opened for writing (curr_file) determinde
     *       by the state
     */
    bool write_line_to_file(const char* line);
    /**
     * Writes a 512 byte buffer to the file 
     * \returns the amount of bytes written to the file
     * \note there is no need path provided, as this
     *       function assumes there is already a file
     *       opened for writing (curr_file) determined
     *       by the state.
     * \note the block written to the file is the tail of the bfr
     * 
     *  The buffer is a member of the object
     */
    uint32_t write_block_to_file();
    /**
     * Reads and clears the error state
     * \returns the error
     * \note this sets the state to idle again
     */
    uint8_t read_clear_error();
    /**
     * Closes the current file
     * \returns none
     */
    void close_file();



    
};



#endif