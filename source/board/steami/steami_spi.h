#pragma once

#include "stm32f1xx_hal.h"
#include "stm32f1xx_hal_spi.h"

void steami_spi_set_handler(SPI_HandleTypeDef* hspi);
HAL_StatusTypeDef steami_spi_init();
HAL_StatusTypeDef steami_spi_deinit();

HAL_StatusTypeDef spi_steami_transfer_receive(uint8_t* tx_buffer, uint8_t* rx_buffer, uint16_t buffer_size, uint32_t timeout);
HAL_StatusTypeDef spi_steami_transfer(uint8_t* data, uint16_t data_size, uint32_t timeout);
