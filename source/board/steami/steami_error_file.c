#include "steami_error_file.h"
#include "virtual_fs.h"
#include "string.h"

char* error_content = "";


static uint32_t steami_error_file_read_callback(uint32_t sector_offset, uint8_t* data, uint32_t num_sector){

    uint32_t read_pos = VFS_SECTOR_SIZE * sector_offset;
    uint32_t file_size = steami_error_file_size();

    if( read_pos < file_size ){
        for( uint32_t i = 0; (read_pos+ i)  < file_size; ++i ){
            data[i] = error_content[read_pos + i];
        }
    }

    return VFS_SECTOR_SIZE;
}

uint32_t steami_error_file_size(){
    return strlen(error_content);
}

void steami_error_file_set_content(char* content){
    error_content = content;
}

void steami_error_file_create_file(char* filename){
    if( error_content != ""){
        vfs_create_file(filename, steami_error_file_read_callback, 0, steami_error_file_size());
    }
}