#pragma once

#include "stdint.h"
#include "stdbool.h"

bool w25q64_init();

void w25q64_hard_reset();

uint8_t w25q64_read_device_id();
uint8_t w25q64_read_manufacturer_id();

/**
 * @brief BUSY is a read only bit in the status register (S0) that is set to a 1 state when the device is executing a Page Program, Sector Erase, Block Erase, Chip Erase, Write Status Register or Erase/Program Security Register instruction. During this time the device will ignore further instructions except for the Read Status Register and Erase/Program Suspend instruction. When the program, erase or write status/security register instruction has completed, the BUSY bit will be cleared to a 0 state indicating the device is ready for further instructions.
 * 
 * @return true if BUSY flag is set, false otherwise
 */
bool w25q64_is_busy();

/**
 * @brief Write Enable Latch (WEL) is a read only bit in the status register (S1) that is set to 1 after executing a Write Enable Instruction. The WEL status bit is cleared to 0 when the device is write disabled. A write disable state occurs upon power-up or after any of the following instructions: Write Disable, Page Program, Quad Page Program, Sector Erase, Block Erase, Chip Erase, Write Status Register, Erase Security Register and Program Security Register.
 * 
 * @return true if WEL flag is set, false otherwise
 */
bool w25q64_is_WEL();

uint8_t w25q64_read_status_register_1();
uint8_t w25q64_read_status_register_2();
uint8_t w25q64_read_status_register_3();

/**
 * @brief Write Enable Latch (WEL) is a read only bit in the status register (S1) that is set to 1 after executing a Write Enable Instruction. The WEL status bit is cleared to 0 when the device is write disabled. A write disable state occurs upon power-up or after any of the following instructions: Write Disable, Page Program, Quad Page Program, Sector Erase, Block Erase, Chip Erase, Write Status Register, Erase Security Register and Program Security Register.
 * 
 */
void w25q64_write_enable();

/**
 * @brief The Read Data instruction allows one or more data bytes to be sequentially read from the memory.
 * 
 * @param data    array to store data
 * @param address   address to start reading
 * @param read_len  Number of byte to read (max. 256)
 */
bool w25q64_read_data(uint8_t* data, uint32_t address, uint16_t read_len);

/**
 * @brief The Page Program instruction allows from one byte to 256 bytes (a page) of data to be programmed at previously erased (FFh) memory locations.
 * 
 * @param data the data pointer
 * @param address the adresse to start writing (be carefull, if the end of the page is reach before the end of data, the address will wrap to the begin of the page)
 * @param data_len Number of byte to write (max 256)
 */
bool w25q64_page_program(uint8_t* data, uint32_t address, uint16_t data_len);

/**
 * @brief The Sector Erase instruction sets all memory within a specified sector (4K-bytes) to the erased state of all 1s (FFh).
 * 
 * @param address Address of sector to be deleted
 */
bool w25q64_sector_erase(uint32_t address);

/**
 * @brief The Sector Erase instruction sets all memory within a specified sector (32K-bytes) to the erased state of all 1s (FFh).
 * 
 * @param address Address of sector to be deleted
 */
void w25q64_block_32k_erase(uint32_t address);

/**
 * @brief The Sector Erase instruction sets all memory within a specified sector (64K-bytes) to the erased state of all 1s (FFh).
 * 
 * @param address Address of sector to be deleted
 */
void w25q64_block_64k_erase(uint32_t address);

/**
 * @brief The Chip Erase instruction sets **all memory** within the device to the erased state of all 1s (FFh). WARNING: This operation can take from 20 to 100 seconds!
 * 
 */
void w25q64_chip_erase();