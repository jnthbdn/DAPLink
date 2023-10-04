#pragma once

#include "stm32f1xx_hal.h"
#include "stm32f1xx_hal_i2c.h"

void steami_i2c_set_handler(I2C_HandleTypeDef *hi2c);
HAL_StatusTypeDef steami_i2c_init();
HAL_StatusTypeDef steami_i2c_deinit();
