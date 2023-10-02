#pragma once

#include "stm32f1xx_hal.h"


HAL_StatusTypeDef steami_i2c_init();
HAL_StatusTypeDef steami_i2c_start_slave();