#pragma once

#include <string.h>
#include <stdint.h>
#include <stdbool.h>

#include "stm32f1xx.h"
#include "stm32f1xx_hal_uart.h"

#define STEAMI_UART_RX_BUFFER 128
#define STEAMI_UART_TX_BUFFER 512

typedef enum{
    DEC,
    HEX
} STEAMI_UART_BASE;


/**
 * @brief Initialize USART1
 * 
 * @return TRUE if successful, FALSE otherwise
 */
bool steami_uart_init();

/**
 * @brief Add raw data to TX buffer
 * 
 * @param data Raw data array
 * @param size Size of data array
 */
void steami_uart_write_data(uint8_t* data, uint16_t size);

/**
 * @brief Add a string to TX buffer. WARNING: The terminating null character is required, but will not be added to the buffer.
 * 
 * @param str The string
 */
void steami_uart_write_string(char* str);

/**
 * @brief Adds a human-readable number to the string and prints it with the specified base.
 * 
 * @param num The number to add
 * @param base The base (HEXadecimal or DECimal)
 */
void steami_uart_write_number(uint32_t num, STEAMI_UART_BASE base);

/**
 * @brief Send the TX buffer
 * 
 * @return TRUE if transmission successful, FALSE otherwise
 */
bool steami_uart_send();

/**
 * @brief Returns the current size of the TX buffer (i.e. the amount of data to be sent).
 * 
 * @return uint16_t
 */
uint16_t steami_uart_tx_size();