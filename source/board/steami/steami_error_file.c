#include "error_steami_file.h"
#include "virtual_fs.h"
#include "string.h"

char* error_content = "";


static uint32_t read_error_steamy_file(uint32_t sector_offset, uint8_t* data, uint32_t num_sector){

    uint32_t read_pos = VFS_SECTOR_SIZE * sector_offset;
    uint32_t file_size = size_steami_error_file();

    if( read_pos < file_size ){
        for( uint32_t i = 0; (read_pos+ i)  < file_size; ++i ){
            data[i] = error_content[read_pos + i];
        }
    }

    return VFS_SECTOR_SIZE;
}

uint32_t size_steami_error_file(){
    return strlen(error_content);
}

void set_steami_error_content(char* content){
    error_content = content;
}

void create_steami_error_file(char* filename){
    if( error_content != ""){
        vfs_create_file(filename, read_error_steamy_file, 0, size_steami_error_file());
    }
}