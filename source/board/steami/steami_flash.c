#include "steami_flash.h"
#include "w25q64.h"

#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "virtual_fs.h"
#include "stm32f1xx_hal.h"

#define STEAMI_FLASH_FILE_ADDR  0x00000000
#define STEAMI_FLASH_NAME_ADDR  0x7FF000

#define STEAMI_FLASH_FILE_SIZE  8384512
#define STEAMI_FLASH_NAME_SIZE  11

#define STEAMI_FALSH_4K   4096
#define STEAMI_FALSH_32K  32768
#define STEAMI_FALSH_64K  65536

static char* user_filename = NULL;
static uint8_t* stream_buffer = NULL;

void wait_w25q64_busy(){
    while(w25q64_is_busy()){
        HAL_Delay(1);
    }
}

void wait_w25q64_wel(){
    while(!w25q64_is_WEL()){
        HAL_Delay(1);
    }
}

static uint32_t steami_read_file(uint32_t sector_offset, uint8_t* data, uint32_t num_sector){

    UNUSED(num_sector);

    uint32_t read_offset = sector_offset * VFS_SECTOR_SIZE;

    for(uint16_t i = 0; i < VFS_SECTOR_SIZE / STEAMI_FLASH_MAX_DATA_SIZE; i++){
        uint16_t available = steami_flash_read_file(stream_buffer, 20, read_offset);
        if( available > 0 ){
            for( uint16_t j = 0; j < available; j++ ){
                data[(VFS_SECTOR_SIZE * i) + j] = stream_buffer[j];
            }
        }

        read_offset += STEAMI_FLASH_MAX_DATA_SIZE;
    }

    return VFS_SECTOR_SIZE;
}

bool steami_flash_init(){
    user_filename = (char*)malloc(11 * sizeof(char));
    stream_buffer = (char*)malloc(STEAMI_FLASH_MAX_DATA_SIZE * sizeof(uint8_t));


    if( user_filename == NULL || stream_buffer == NULL ){
        return false;
    }

    memset(user_filename, '\0', 11);
    memset(stream_buffer, '\0', STEAMI_FLASH_MAX_DATA_SIZE);

    return true;
}

void steami_flash_create_file(){
    if( steami_flash_get_size() > 0 ){
        steami_flash_get_filename(user_filename);
        vfs_create_file(user_filename, steami_read_file, NULL, steami_flash_get_size());
    }
}

void steami_flash_erase(){
    uint32_t size = steami_flash_get_size();
    uint32_t offset = 0;

    while( size > 0 ){
        wait_w25q64_busy();
        w25q64_write_enable();
        wait_w25q64_busy();

        if( size >= STEAMI_FALSH_64K ){
            w25q64_block_64k_erase(STEAMI_FLASH_FILE_ADDR + offset);

            offset += STEAMI_FALSH_64K;
            size -= STEAMI_FALSH_64K;
        }
        else if (size >= STEAMI_FALSH_32K ){
            w25q64_block_32k_erase(STEAMI_FLASH_FILE_ADDR + offset);

            offset += STEAMI_FALSH_32K;
            size -= STEAMI_FALSH_32K;
        }
        else{
            w25q64_sector_erase(STEAMI_FLASH_FILE_ADDR + offset);
            
            offset += STEAMI_FALSH_4K;
            size -= (size >= STEAMI_FALSH_4K) ? STEAMI_FALSH_4K : size;
        }
    }
}

bool steami_flash_set_filename(char* filename, char* ext){
    char new_filename[STEAMI_FLASH_NAME_SIZE + 1];
    char current_filename[STEAMI_FLASH_NAME_SIZE + 1];

    steami_flash_get_filename(current_filename);
    current_filename[STEAMI_FLASH_NAME_SIZE] = '\0';

    sprintf(new_filename, "%-8.8s%-3.3s", filename, ext);

    for(uint8_t i = 0; i < STEAMI_FLASH_NAME_SIZE; ++i) { 
        new_filename[i] = toupper(new_filename[i]);
    }

    if( ! filename_valid(new_filename) ){
        return false;
    }

    if( strcmp(current_filename, new_filename) == 0){
        return false;
    }

    wait_w25q64_busy();
    w25q64_write_enable();
    wait_w25q64_wel();

    wait_w25q64_busy();
    w25q64_sector_erase(STEAMI_FLASH_NAME_ADDR);

    wait_w25q64_busy();
    w25q64_write_enable();
    wait_w25q64_wel();

    wait_w25q64_busy();
    w25q64_page_program(new_filename, STEAMI_FLASH_NAME_ADDR, STEAMI_FLASH_NAME_SIZE);

    return true;
}

void steami_flash_get_filename(char* filename){ 
    
    memset(filename, '\0', STEAMI_FLASH_NAME_SIZE);
    
    wait_w25q64_busy();
    w25q64_read_data(filename, STEAMI_FLASH_NAME_ADDR, STEAMI_FLASH_NAME_SIZE);
}

uint32_t steami_flash_get_size(){
    uint8_t data[STEAMI_FLASH_MAX_DATA_SIZE];
    memset(data, 0xFF, STEAMI_FLASH_MAX_DATA_SIZE);

    for( uint32_t offset = 0; offset < STEAMI_FLASH_FILE_SIZE; offset += STEAMI_FLASH_MAX_DATA_SIZE ){
        wait_w25q64_busy();
        w25q64_read_data(data, STEAMI_FLASH_FILE_ADDR + offset, STEAMI_FLASH_MAX_DATA_SIZE);

        for(uint16_t i = 0; i < STEAMI_FLASH_MAX_DATA_SIZE; ++i){
            if( data[i] == 0xFF ){
                return STEAMI_FLASH_FILE_ADDR + offset + i;
            }
        }
        
    }

    return STEAMI_FLASH_FILE_SIZE;
}

void steami_flash_append_file(uint8_t* data, uint16_t data_len){
    uint32_t size = steami_flash_get_size();

    if( size >= STEAMI_FLASH_FILE_SIZE ){ return; }

    wait_w25q64_busy();
    w25q64_write_enable();
    wait_w25q64_busy();
    w25q64_page_program(data, STEAMI_FLASH_FILE_ADDR + size, data_len);
}

uint16_t steami_flash_read_file(uint8_t* data, uint16_t data_len, uint32_t offset){
    uint16_t filesize = steami_flash_get_size();

    if( offset >= filesize ){
        return 0;
    }

    uint16_t available = filesize - offset;
    uint16_t data_to_read = (available >= data_len) ? data_len : available;

    if( data_to_read == 0 ){
        return 0;
    }

    wait_w25q64_busy();
    w25q64_read_data(data, STEAMI_FLASH_FILE_ADDR + offset, data_to_read);

    return data_to_read;
}