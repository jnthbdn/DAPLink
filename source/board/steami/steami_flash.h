#pragma once

#include "stdint.h"
#include "stdbool.h"

bool steami_flash_init();

uint8_t read_device_id();
uint8_t read_manufacturer_id();
uint8_t read_status();