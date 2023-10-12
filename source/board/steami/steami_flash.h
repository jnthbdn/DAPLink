#pragma once

#include <stdint.h>
#include <stdbool.h>

#define STEAMI_FLASH_MAX_DATA_SIZE 256

bool steami_flash_init();
void steami_flash_create_file();

void steami_flash_erase();
bool steami_flash_set_filename(char* filename, char* ext);
void steami_flash_get_filename(char* filename);
uint32_t steami_flash_get_size();
void steami_flash_append_file(uint8_t* data, uint16_t data_len);

/**
 * @brief Read the file from flash memory.
 * 
 * @param data array for storing read data
 * @param data_len array size (max 256)
 * @param offset number of byte to skip before reading the file
 * @return uint16_t number of bytes read
 */
uint16_t steami_flash_read_file(uint8_t* data, uint16_t data_len, uint32_t offset);