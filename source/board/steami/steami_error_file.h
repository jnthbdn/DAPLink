#pragma once

#include "stdint.h"

uint32_t steami_error_file_size();
void steami_error_file_set_content(char* content);
void steami_error_file_create_file(char* filename);