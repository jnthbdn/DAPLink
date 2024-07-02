#pragma once

#include <stdint.h>
#include <stdbool.h>

#define STEAMI_FLASH_FILE_ADDR  (uint32_t)0x00000000
#define STEAMI_FLASH_NAME_ADDR  (uint32_t)0x007FF000

#define STEAMI_FLASH_FILE_SIZE  8384512

#define STEAMI_FLASH_SECTOR 256
#define STEAMI_FLASH_4K   4096
#define STEAMI_FLASH_32K  32768
#define STEAMI_FLASH_64K  65536

#define STEAMI_FLASH_NB_SECTOR 32768

#define STEAMI_FLASH_MAX_DATA_SIZE STEAMI_FLASH_SECTOR
#define STEAMI_FLASH_NAME_SIZE  11

/**
 * @brief Initialize the flash chip
 * 
 * @return TRUE if successful, FALSE otherwise
 */
bool steami_flash_init();

/**
 * @brief Create file in the DapLink VFS 
 * 
 */
void steami_flash_create_file();

/**
 * @brief Erase file content
 * 
 */
void steami_flash_erase();

/**
 * @brief Erase the sector containing the file name
 * 
 * @return TRUE if successful, FALSE otherwise
 */
bool steami_flash_erase_filename_sector();

/**
 * @brief 
 * 
 * @param filename Write the string in the sector containing the filename
 * 
 * @return TRUE if successful, FALSE otherwise
 */
bool steami_flash_set_filename(char* filename);

/**
 * @brief Read the data in the sector containing the filename
 * 
 * @param filename The buffer to store the filename. WARNING: The table must have a minimum size of 11 charaters. No terminating null character will be added.
 */
void steami_flash_get_filename(char* filename);

/**
 * @brief Get the file name currently displayed in the mounted device (may be different from the current file name in flash), this file name only changes after DapLink restarts.
 * 
 * @param filename The buffer to store the filename. WARNING: The table must have a minimum size of 11 charaters. No terminating null character will be added.
 */
void steami_flash_get_filename_mount(char* filename);

/**
 * @brief Get file size in flash
 * 
 * @return uint32_t
 */
uint32_t steami_flash_get_size();

/**
 * @brief Add data at the end of the file
 * 
 * @param data Data array
 * @param data_len size of data array
 * @return int16_t 
 */
int16_t steami_flash_append_file(uint8_t* data, uint16_t data_len);

/**
 * @brief 
 * 
 * @param sector_number 
 * @param data 
 * @return true 
 * @return false 
 */
bool steami_flash_read_sector(uint32_t sector_number, uint8_t* data);

/**
 * @brief Read the file from flash memory.
 * 
 * @param data array for storing read data
 * @param data_len array size (max 256)
 * @param offset number of byte to skip before reading the file
 * @return uint16_t number of bytes read
 */
uint16_t steami_flash_read_file(uint8_t* data, uint16_t data_len, uint32_t offset);

/**
 * @brief Get if the flash is busy
 * 
 * @return TRUE if the flash is buzy flag is high, FALSE otherwise 
 */
bool steami_flash_is_busy();