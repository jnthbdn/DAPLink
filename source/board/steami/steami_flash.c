#include "steami_flash.h"
#include "w25q64.h"

#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#include "virtual_fs.h"
#include "stm32f1xx_hal.h"


static char user_filename[STEAMI_FLASH_NAME_SIZE] = {'\0'};
static uint8_t stream_buffer[STEAMI_FLASH_MAX_DATA_SIZE] = {0};

void wait_w25q64_busy(){
    while(w25q64_is_busy());
}

void wait_w25q64_wel(){
    while(!w25q64_is_WEL());
}

static uint32_t steami_read_file(uint32_t sector_offset, uint8_t* data, uint32_t num_sector){

    UNUSED(num_sector);

    uint32_t read_offset = sector_offset * VFS_SECTOR_SIZE;

    for(uint16_t i = 0; i < VFS_SECTOR_SIZE / STEAMI_FLASH_MAX_DATA_SIZE; i++){
        uint16_t available = steami_flash_read_file(stream_buffer, STEAMI_FLASH_MAX_DATA_SIZE, read_offset);
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
    memset(user_filename, '\0', STEAMI_FLASH_NAME_SIZE);
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
 
        if( size >= STEAMI_FLASH_64K ){
            w25q64_block_64k_erase(STEAMI_FLASH_FILE_ADDR + offset);

            offset += STEAMI_FLASH_64K;
            size -= STEAMI_FLASH_64K;
        }
        else if (size >= STEAMI_FLASH_32K ){
            w25q64_block_32k_erase(STEAMI_FLASH_FILE_ADDR + offset);

            offset += STEAMI_FLASH_32K;
            size -= STEAMI_FLASH_32K;
        }
        else{
            w25q64_sector_erase(STEAMI_FLASH_FILE_ADDR + offset);
            
            offset += STEAMI_FLASH_4K;
            size -= (size >= STEAMI_FLASH_4K) ? STEAMI_FLASH_4K : size;
        }
    }
}

bool steami_flash_erase_filename_sector(){

    w25q64_write_enable();
    wait_w25q64_wel();

    wait_w25q64_busy();
    return w25q64_sector_erase(STEAMI_FLASH_NAME_ADDR);
}

bool steami_flash_set_filename(char* filename){
    w25q64_write_enable();
    wait_w25q64_wel();

    wait_w25q64_busy();
    return w25q64_page_program(filename, STEAMI_FLASH_NAME_ADDR, STEAMI_FLASH_NAME_SIZE);
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
        w25q64_read_data(data, STEAMI_FLASH_FILE_ADDR + offset, STEAMI_FLASH_MAX_DATA_SIZE);

        for(uint16_t i = 0; i < STEAMI_FLASH_MAX_DATA_SIZE; ++i){
            if( data[i] == 0xFF ){
                return STEAMI_FLASH_FILE_ADDR + offset + i;
            }
        }
        
    }

    return STEAMI_FLASH_FILE_SIZE;
}

int16_t steami_flash_append_file(uint8_t* data, uint16_t data_len){
    uint32_t offset = steami_flash_get_size();
    uint8_t block = offset / STEAMI_FLASH_64K;
    uint16_t address_block = offset % STEAMI_FLASH_64K;
    uint16_t page = address_block / 256;
    uint16_t offset_page = address_block % 256;
    uint16_t available_space = 256 - offset_page;

    if( available_space > data_len ){
        available_space = data_len;
    }


    w25q64_write_enable();
    wait_w25q64_wel();

    wait_w25q64_busy();
    if( w25q64_page_program(data, offset, available_space) ){
        return available_space;
    }
    else{
        return -1;
    }
}

bool steami_flash_read_sector(uint32_t sector_number, uint8_t* data){

    if( sector_number >= STEAMI_FLASH_NB_SECTOR ){
        return false;
    }

    return w25q64_read_data(data, 0, 256);
}

uint16_t steami_flash_read_file(uint8_t* data, uint16_t data_len, uint32_t offset){
    uint32_t filesize = steami_flash_get_size();

    if( offset >= filesize ){
        return 0;
    }

    uint32_t available_to_read = filesize - offset;
    uint32_t data_to_read = (available_to_read >= data_len) ? data_len : available_to_read;

    if( data_to_read == 0 ){
        return 0;
    }

    wait_w25q64_busy();
    w25q64_read_data(data, STEAMI_FLASH_FILE_ADDR + offset, data_to_read);

    return data_to_read;
}

bool steami_flash_is_busy(){
    return w25q64_is_busy();
}