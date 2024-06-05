#pragma once

#include <stdbool.h>

#include "stm32f1xx.h"
#include "stm32f1xx_hal_i2c.h"


#define STEAMI_I2C_TX_BUFFER_SIZE 512
#define STEAMI_I2C_RX_BUFFER_SIZE 256

#define I2C_STEAMI_ADDRESS 0X76

#define I2C_STEAMI_MASTER_READ_TIMEOUT_MS 500


typedef enum  {
    UNKNOWN = 0x00,

    WHO_AM_I = 0x01,
    
    SET_FILENAME = 0x03,
    GET_FILENAME = 0x04,

    CLEAR_FLASH = 0x10,
    WRITE_DATA = 0x11,

    READ_SECTOR = 0x20,
    
    STATUS = 0x80,
    ERROR_STATUS = 0x81,
} steami_i2c_command;

typedef void (*steami_cmd_callback)(steami_i2c_command cmd, uint8_t* rx_data, uint16_t rx_len);

/**
 * @brief Initialize I2C2
 * 
 * @return TRUE if successful, FALSE otherwise
 */
bool steami_i2c_init();

/**
 * @brief Deinit I2C2
 * 
 */
void steami_i2c_deinit();

/**
 * @brief Set the function to be called each time a valid command is received (via I2C)
 * 
 * @param callback The callback returns the number of bytes to be returned (maximum data is defined by STEAMI_I2C_TX_BUFFER_SIZE). `cmd` is the command received. `rx_data` contains the data received (without the command). `len_rx` is the number of bytes received. `tx_data` contains the data to be sent back to the master.
 */
void steami_i2c_on_receive_command(steami_cmd_callback callback);

/**
 * @brief Set the data to sent to the master on request
 * 
 * @param data data array
 * @param len size of data array
 */
void steami_i2c_set_tx_data(uint8_t* data, uint16_t len);

/**
 * @brief This function need to be call periodically
 * 
 */
void steami_i2c_process();