/**
 * @file    steamy32.c
 * @brief   board ID for the STeamy32 board
 *
 * DAPLink Interface Firmware
 * Copyright (c) 2009-2019, ARM Limited, All Rights Reserved
 * SPDX-License-Identifier: Apache-2.0
 *
 * Licensed under the Apache License, Version 2.0 (the "License"); you may
 * not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS, WITHOUT
 * WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */


/**
# I2C Flash

In addition to DapLink, STeaMi's STM32F103 includes an I2C “peripheral” for communication with Flash. This makes it possible to store data in a “file” that can be accessed via the mass storage created by DapLink when plugged into a computer's USB port.

## I2C configuration
The F103's I2C peripheral used to communicate with the WB55 is the `I2C2`, using pins `PB_10` and `PB_11`. The F103's 7-bit address is `0x76` for writing, and `0x77` for reading. The bus speed is set to `100 kHz` and the “No Stech Mode” is active.

## Available command

Here's a table summarizing the commands available via I2C, their description, parameters and results.

| Name            | 8-bits value | parameters      | Result     | Description                                                                                                                                                                                                                                                           |
| --------------- | ------------ | --------------- | ---------- | --------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------- |
| Who Am I        | 0x01         | _NONE_          | 1 octet    | Get the device ID (0x4C)                                                                                                                                                                                                                                              |
| Set filename    | 0x03         | 11 octets       | _NONE_     | Defines the file name, using the 8.3 naming standard ([https://en.wikipedia.org/wiki/8.3_filename](https://en.wikipedia.org/wiki/8.3_filename)). If the file name (or extension) is less than 8 characters (respectively, 3 characters), use spaces to fill (`0x20`). |
| Get filename    | 0x04         | _NONE_          | 11 octets  | Get the file name.                                                                                                                                                                                                                                                    |
| Clear Flash     | 0x10         | _NONE_          | _NONE_     | Erase file content.                                                                                                                                                                                                                                                   |
| Write data      | 0x11         | 256 octets max. | _NONE_     | Append data to file.                                                                                                                                                                                                                                                  |
| Read sector     | 0x20         | 1 octet         | 256 octets | Read a sector (the parameters must be between 0-32768)                                                                                                                                                                                                                |
| Status Register | 0x80         | _NONE_          | 1 octet    | Get the status register (see below)                                                                                                                                                                                                                                   |
| Error Register  | 0x81         | _NONE_          | 1 octet    | Get the error register (see below)                                                                                                                                                                                                                                    |

## Status & Error registers
### Status Register
The Status Register provides information on the status of the device.
| Bit | Name       | Description                                                                                                                                                   |
| --- | ---------- | ------------------------------------------------------------------------------------------------------------------------------------------------------------- |
| 0   | _RESERVED_ |                                                                                                                                                               |
| 1   | _RESERVED_ |                                                                                                                                                               |
| 2   | _RESERVED_ |                                                                                                                                                               |
| 3   | _RESERVED_ |                                                                                                                                                               |
| 4   | _RESERVED_ |                                                                                                                                                               |
| 5   | _RESERVED_ |                                                                                                                                                               |
| 6   | _RESERVED_ |                                                                                                                                                               |
| 7   | BUSY       | This bit is set to 1 when the device is processing a task. No commands will be processed except “Who Am I”, “Read Status Register” and “Read Error Register”. |

### Error Register
The error register provides information on errors encountered by the device.
| Bit | Name                | Description                                                                             |
| --- | ------------------- | --------------------------------------------------------------------------------------- |
| 0   | Bad Parameters      | Parameters sent after command are invalid (For exemple: Too short or too long filename) |
| 1   | _RESERVED_          |                                                                                         |
| 2   | _RESERVED_          |                                                                                         |
| 3   | _RESERVED_          |                                                                                         |
| 4   | _RESERVED_          |                                                                                         |
| 5   | File is Full        | The file has reached its maximum size, and no more data can be added.                   |
| 6   | Bad Filename        | The filename contains invalid characters                 0x00000020                               |
| 7   | Last Command Failed | The last command failed                                                                 |
*/
#ifdef CMSIS_DAP_PRODUCT_NAME
#error "CMSIS_DAP_PRODUCT_NAME already defined"
#endif
#define CMSIS_DAP_PRODUCT_NAME "STeaMi"

#include "main_interface.h"
#include "target_family.h"
#include "target_board.h"
#include "rtx_os.h"

#include "stm32f1xx.h"
#include "stm32f1xx_hal_gpio.h"
#include "stm32f1xx_hal_rcc.h"

#include "steami_error_file.h"
#include "steami_i2c.h"
#include "steami_spi.h"
#include "steami_uart.h"
#include "w25q64.h"
#include "steami_flash.h"
#include "error_status.h"
#include "steami_tim.h"

#include <stdlib.h>
#include <ctype.h>

typedef enum {
    TASK_NONE,

    TASK_WHO_AM_I,
    
    TASK_CLEAR_FLASH,
    
    TASK_SET_FILENAME_CLEAR,
    TASK_SET_FILNAME_WRITE,
    TASK_GET_FILENAME,
    
    TASK_WRITE_DATA_COUNT,
    TASK_WRITE_DATA_WRITE,

    TASK_READ_SECTOR,

    TASK_WAIT_FLASH_BUSY,
} steami_task;


static steami_task current_task = TASK_NONE;
static char buffer_filename[STEAMI_FLASH_NAME_SIZE] = {'\0'};
static uint8_t buffer_sector[STEAMI_FLASH_SECTOR] = {0};
static uint8_t task_rx[STEAMI_I2C_RX_BUFFER_SIZE] = {'\0'};
static uint16_t task_rx_len = 0;
static uint8_t status_error = 0;

static bool is_busy(){ return current_task != TASK_NONE; }

static void steami_init_led(){
    GPIO_InitTypeDef led = {0};
    led.Pin = GPIO_PIN_6;
    led.Mode = GPIO_MODE_OUTPUT_PP;
    led.Pull = GPIO_NOPULL;
    led.Speed = GPIO_SPEED_FREQ_HIGH;

    HAL_GPIO_Init(GPIOA, &led);

    led.Pin = GPIO_PIN_1;
    HAL_GPIO_Init(GPIOB, &led);

    HAL_GPIO_WritePin(GPIOA, GPIO_PIN_6, GPIO_PIN_SET);
    HAL_GPIO_WritePin(GPIOB, GPIO_PIN_1, GPIO_PIN_SET);
}

static void toggle_led_blue(){
    HAL_GPIO_TogglePin(GPIOA, GPIO_PIN_6);
}

static void toggle_led_green(){
    HAL_GPIO_TogglePin(GPIOB, GPIO_PIN_1);
}

static void on_I2C_receive_command(steami_i2c_command cmd, uint8_t* rx, uint16_t rx_len){

    // toggle_led_blue();

    if( rx_len > STEAMI_I2C_RX_BUFFER_SIZE ){
        rx_len = STEAMI_I2C_RX_BUFFER_SIZE;
    }

    steami_uart_write_string("I2C Command: ");
    steami_uart_write_number(cmd, HEX);
    steami_uart_write_string(" (rx_len: ");
    steami_uart_write_number(rx_len, DEC);
    steami_uart_write_string(")\n");

    switch(cmd){
        case WHO_AM_I:{
            uint8_t id = 0x4C;
            steami_i2c_set_tx_data(&id, 1);
            break;
        }

        case CLEAR_FLASH:
            if(is_busy()){
                steami_uart_write_string("ERROR I2C busy.\n");
                break;
            }

            current_task = TASK_CLEAR_FLASH;
            break;

        case SET_FILENAME: {
            if(is_busy()){
                steami_uart_write_string("ERROR I2C busy.\n");
                break;
            }

            current_task = TASK_SET_FILENAME_CLEAR;
            memcpy(task_rx, rx, rx_len);
            task_rx_len = rx_len;
            break;
        }

        case GET_FILENAME:
            if(is_busy()){
                steami_uart_write_string("ERROR I2C busy.\n");
                break;
            }

            memset(buffer_filename, 0x00, STEAMI_FLASH_NAME_SIZE);
            current_task = TASK_GET_FILENAME;
            break;

        case WRITE_DATA:
            if(is_busy()){
                steami_uart_write_string("ERROR I2C busy.\n");
                break;
            }

            current_task = TASK_WRITE_DATA_COUNT;
            memcpy(task_rx, rx, rx_len);
            task_rx_len = rx_len;
            break;
        
        case READ_SECTOR:{
            if(is_busy()){
                steami_uart_write_string("ERROR I2C busy.\n");
                break;
            }

            current_task = TASK_READ_SECTOR;
            memcpy(task_rx, rx, rx_len);
            task_rx_len = rx_len;
            break;
        }

        case STATUS:{
            uint8_t status = 0x00;

            if( is_busy() ){
                status |= 0x80;
            }

            steami_i2c_set_tx_data(&status, 1);
            break;
        }

        case ERROR_STATUS:
            steami_i2c_set_tx_data(&status_error, 1);
            error_status_clear(&status_error);
            break;
        
        default:
            error_status_set_last_command_fail(&status_error);
            break;
    }

    error_status_reset_last_command_fail(&status_error);
}

void process_task()
{
    // toggle_led_green();

    steami_i2c_process();
    steami_uart_send();

    if(current_task != TASK_NONE){

        steami_uart_write_string("Process task: ");
        steami_uart_write_number(current_task, HEX);
        steami_uart_write_string("\n");

        switch(current_task){

            case TASK_WHO_AM_I: {
                uint8_t id = 0x4C;
                steami_i2c_set_tx_data(&id, 1);
                current_task = TASK_NONE;
                break;
            }

            case TASK_CLEAR_FLASH:
                if( steami_flash_is_busy() ){
                    steami_uart_write_string("ERROR flash busy.\n");
                    break;
                }

                steami_flash_erase();
                error_status_reset_file_full(&status_error);
                current_task = TASK_WAIT_FLASH_BUSY;
                break;

            case TASK_GET_FILENAME:
                if( steami_flash_is_busy() ){
                    steami_uart_write_string("ERROR flash busy.\n");
                    break;
                }
                
                steami_flash_get_filename(buffer_filename);
                steami_i2c_set_tx_data(buffer_filename, 11);

                steami_uart_write_string("Filename: ");
                steami_uart_write_data(buffer_filename, 11);
                steami_uart_write_string("\n");

                current_task = TASK_WAIT_FLASH_BUSY;
                break;

            case TASK_SET_FILENAME_CLEAR: {
                if( steami_flash_is_busy() ){
                    steami_uart_write_string("ERROR flash busy.\n");
                    break;
                }
                
                if( task_rx_len != 11 ){
                    error_status_bad_parameter(&status_error);
                    steami_uart_write_string("ERROR Filename must have 11 chars.\n");
                    current_task = TASK_NONE;
                    break;
                }

                if( ! filename_valid(task_rx) ){
                    error_status_set_bad_filename(&status_error);
                    steami_uart_write_string("ERROR Invalid filename.\n");
                    current_task = TASK_NONE;
                    break;
                }

                if (!steami_flash_erase_filename_sector()){
                    error_status_set_last_command_fail(&status_error);
                    steami_uart_write_string("ERROR while deleting filename sector.\n");
                    current_task = TASK_NONE;
                    break;
                }

                current_task = TASK_SET_FILNAME_WRITE;
                break;
            }

            case TASK_SET_FILNAME_WRITE: {
                if( steami_flash_is_busy() ){
                    break;
                }

                if (!steami_flash_set_filename(task_rx)){
                    error_status_set_last_command_fail(&status_error);
                    steami_uart_write_string("ERROR Unable to write filename in flash.\n");
                }

                current_task = TASK_WAIT_FLASH_BUSY;
                break;
            }

            case TASK_WRITE_DATA_COUNT: {
                uint32_t current_size = steami_flash_get_size();

                if( current_size == STEAMI_FLASH_FILE_SIZE ){
                    error_status_set_file_full(&status_error);
                    steami_uart_write_string("ERROR flash file is full.\n");
                    current_task = TASK_NONE;
                }
                else{
                    current_task = TASK_WRITE_DATA_WRITE;
                }
                break;
            }

            case TASK_WRITE_DATA_WRITE:{
                if( steami_flash_is_busy() ){
                    steami_uart_write_string("ERROR flash busy.\n");
                    break;
                }

                if( task_rx_len == 0){
                    steami_uart_write_string("No more data in rx_len to write to flash.\n");
                    current_task = TASK_NONE;
                    break;
                }

                uint32_t bytes_wrote = 0;
                int16_t bytes_sent = steami_flash_append_file(task_rx + bytes_wrote, task_rx_len);

                if( bytes_sent == -1 ){
                    error_status_set_last_command_fail(&status_error);
                    steami_uart_write_string("ERROR Unable to write data to flash\n");
                    current_task = TASK_NONE;
                }
                else{
                    task_rx_len -= bytes_sent;
                    bytes_wrote += bytes_sent;
                    current_task = TASK_WRITE_DATA_WRITE;
                }
                break;
            }

            case TASK_READ_SECTOR:{

                if( task_rx_len == 1 && steami_flash_read_sector(task_rx[0], buffer_sector) ){
                    steami_i2c_set_tx_data(buffer_sector, 256);
                }
                else{
                    error_status_set_last_command_fail(&status_error);
                    steami_uart_write_string("ERROR Unable to read sector (bad sector parameter ?)\n");
                }

                current_task = TASK_WAIT_FLASH_BUSY;
                break;
            }


            case TASK_WAIT_FLASH_BUSY:
                if( !steami_flash_is_busy() ){
                    current_task = TASK_NONE;
                }
                break;

            default:
                current_task = TASK_NONE;
                steami_uart_write_string("ERROR Unknown task.\n");
                break;
        }
    }
}

void vfs_user_build_filesystem_hook(){
    steami_flash_create_file();
    steami_error_file_create_file("STMI_ERRTXT");
}

static void prerun_board_config(void)
{
    steami_i2c_on_receive_command(on_I2C_receive_command);
    
    if(! steami_tim_init(10) ){
        steami_error_file_set_content( "TIM init failed");
        return;
    }

    if( ! steami_uart_init() ){
        steami_error_file_set_content( "UART init failed");
        return;
    }

    if( ! steami_i2c_init() ){
        steami_error_file_set_content("I2C init failed !");
        steami_uart_write_string("I2C init failed !");
        return;
    }

    if( ! steami_spi_init() ){
        steami_error_file_set_content("SPI init failed !");
        steami_uart_write_string("SPI init failed !");
        return;
    }

    if( ! w25q64_init() ){
        steami_error_file_set_content( "Flash init failed !");
        steami_uart_write_string( "Flash init failed !");
        return;
    }

    if( ! steami_flash_init() ){
        steami_error_file_set_content( "User file init failed !");
        steami_uart_write_string( "User file init failed !");
        return;
    }

    steami_init_led();

    steami_tim_set_callback(process_task);
    steami_uart_write_string("STeaMi DapLink: Ready\n");
}

uint32_t osRtxErrorNotify (uint32_t code, void *object_id){

    switch (code) {
        case osRtxErrorStackUnderflow:
            // Stack overflow detected for thread (thread_id=object_id)
            steami_uart_write_string("[OS RTX ERROR]: Stack Overflow (thread_id=");
            steami_uart_write_number(*((uint32_t*)object_id), HEX);
            steami_uart_write_string(")\n");
            break;

        case osRtxErrorISRQueueOverflow:
            // ISR Queue overflow detected when inserting object (object_id)
            steami_uart_write_string("[OS RTX ERROR]: ISR Queue Overflow\n");
            break;

        case osRtxErrorTimerQueueOverflow:
            // User Timer Callback Queue overflow detected for timer (timer_id=object_id)
            steami_uart_write_string("[OS RTX ERROR]: Timer Queue Overflow\n");
            break;

        case osRtxErrorClibSpace:
            // Standard C/C++ library libspace not available: increase OS_THREAD_LIBSPACE_NUM
            steami_uart_write_string("[OS RTX ERROR]: Standard C/C++ library libspace not available: increase OS_THREAD_LIBSPACE_NUM\n");
            for(;;);
            break;

        case osRtxErrorClibMutex:
            // Standard C/C++ library mutex initialization failed
            steami_uart_write_string("[OS RTX ERROR]: Standard C/C++ library mutex initialization failed\n");
            for(;;);
            break;

        default:
            steami_uart_write_string("[OS RTX ERROR]: Unknown error\n");
        break;
    }
}

const board_info_t g_board_info = {
    .info_version = kBoardInfoVersion,
    .board_id = "0839",
    .family_id = kStub_HWReset_FamilyID,
    .daplink_url_name   = "STEAMI  HTM",
    .daplink_drive_name = "STeaMi     ",
    .daplink_target_url = "https://www.steami.cc/",
    .prerun_board_config = prerun_board_config,
    .target_cfg = &target_device,
    .board_name = "STeaMi",
};
