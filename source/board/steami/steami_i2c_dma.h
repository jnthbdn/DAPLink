#pragma once

#include <stdint.h>
#include <stdbool.h>

#include "stm32f1xx.h"
#include "stm32f1xx_hal_i2c.h"

/**
 * @brief Initialize DMA
 * 
 * @param huart The USART1 Handle attach to DMA
 * @return TRUE if successful, FALSE otherwise
 */
bool steami_i2c_dma_init(I2C_HandleTypeDef * huart);
