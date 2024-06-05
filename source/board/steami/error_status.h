#pragma once

#include <stdint.h>

/***
 * ===== ERROR STATUS =====
 * 
 * +------+------+------+------+------+------+------+------+
 * |  LC  |  R   |  R   |  R   |  R   |  FF  |  BF  |  BP  |
 * +------+------+------+------+------+------+------+------+
 * 
 * Bit 7 - Last Command failure (LC)
 *      If 1 the last command failed
 * 
 * Bit 6 - Reserved
 *      Reserved
 * 
 * Bit 5 - Reserved
 *      Reserved
 * 
 * Bit 4 - Reserved
 *      Reserved
 * 
 * Bit 3 - Reserved
 *      Reserved
 * 
 * Bit 2 - File is Full (FF)
 *      The file has reached its maximum size, and no more data can be added.
 * 
 * Bit 1 - Bad Filename (BF)
 *      The filename contains invalid characters
 * 
 * Bit 0 - Bad Parameters (BP)
 *      Parameters sent after command are invalid
 *      (For exemple: Too short or too long filename)
 * 
*/


void error_status_clear(uint8_t* status){
    *status = 0x00;
}

void error_status_bad_parameter(uint8_t* status){
    *status |= 0x01;
}

void error_status_set_bad_filename(uint8_t* status){
    *status |= 0x02;
}

void error_status_set_file_full(uint8_t* status){
    *status |= 0x04;
}

void error_status_reset_file_full(uint8_t* status){
    *status &= ~0x04;
}

void error_status_set_last_command_fail(uint8_t* status){
    *status |= 0x80;
}

void error_status_reset_last_command_fail(uint8_t* status){
    *status &= ~0x80;
}
