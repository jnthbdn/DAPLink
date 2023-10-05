#pragma once

#include "stdint.h"
#include "stdbool.h"

bool w25q64_init();

uint8_t w25q64_read_device_id();
uint8_t w25q64_read_manufacturer_id();
uint8_t w25q64_read_status();
