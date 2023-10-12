#pragma once

#include "stm32f1xx_hal.h"
#include "stm32f1xx_hal_i2c.h"

#define STEAMI_I2C_MAX_TX_DATA 12

typedef enum  {
    WHO_AM_I = 0x01,
    CLEAR_FLASH = 0x02,
    SET_FILENAME = 0x03,
    GET_FILENAME = 0x04,
    WRITE_DATA = 0x0A
} steami_i2c_command;

void steami_i2c_set_handler(I2C_HandleTypeDef *hi2c);
HAL_StatusTypeDef steami_i2c_init();
HAL_StatusTypeDef steami_i2c_deinit();

/**
 * @brief Set the function to be called each time a valid command is received (via I2C)
 * 
 * @param callback The callback returns the number of bytes to be returned (maximum data is defined by STEAMI_I2C_MAX_TX_DATA). `cmd` is the command received. `rx_data` contains the data received (without the command). `len_rx` is the number of bytes received. `tx_data` contains the data to be sent back to the master.
 */
void steami_i2c_on_receive_command( uint16_t(*callback)(steami_i2c_command cmd, uint8_t* rx_data, uint16_t len_rx, uint8_t* tx_data) );
