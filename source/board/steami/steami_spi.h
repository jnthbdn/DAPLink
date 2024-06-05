#pragma once

#include <stdbool.h>

#include "stm32f1xx.h"
#include "stm32f1xx_hal_spi.h"

/**
 * @brief Initialize SPI1
 * 
 * @return TRUE if successful, FALSE otherwise 
 */
bool steami_spi_init();

/**
 * @brief Transfert and Receive data to/from SPI peripheral
 * 
 * @param tx_buffer Data array to be send
 * @param rx_buffer Data array for storing received data (WARNING: Must be the same size as tx_buffer)
 * @param buffer_size the size of tx_buffer
 * @param timeout timeout (in milliseconds)
 * @return TRUE successful, FALSE otherwise 
 */
bool spi_steami_transfer_receive(uint8_t* tx_buffer, uint8_t* rx_buffer, uint16_t buffer_size, uint32_t timeout);

/**
 * @brief Transfert data to SPI peripheral
 * 
 * @param data Data array to be send
 * @param data_size the size of data array
 * @param timeout timeout (in milliseconds)
 * @return TRUE if successful, FALSE otherwise 
 */
bool spi_steami_transfer(uint8_t* data, uint16_t data_size, uint32_t timeout);
