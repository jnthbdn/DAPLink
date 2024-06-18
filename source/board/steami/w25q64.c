#include "w25q64.h"
#include "steami_spi.h"

#include "stm32f1xx_hal_gpio.h"

#include "string.h"

bool w25q64_init(){
    __HAL_RCC_GPIOB_CLK_ENABLE();

    GPIO_InitTypeDef gpio = {0};

    gpio.Pin = GPIO_PIN_8 | GPIO_PIN_9;
    gpio.Mode = GPIO_MODE_OUTPUT_PP;
    gpio.Pull = GPIO_PULLUP;
    gpio.Speed = GPIO_SPEED_FREQ_HIGH;


    HAL_GPIO_Init(GPIOB, &gpio);

    HAL_GPIO_WritePin(GPIOB, GPIO_PIN_8, GPIO_PIN_SET);
    HAL_GPIO_WritePin(GPIOB, GPIO_PIN_9, GPIO_PIN_SET);
    
    w25q64_hard_reset();
    
    return w25q64_read_device_id() == 0x16;
}

void w25q64_hard_reset(){
    HAL_Delay(5);
    HAL_GPIO_WritePin(GPIOB, GPIO_PIN_9, GPIO_PIN_RESET);
    HAL_Delay(5);
    HAL_GPIO_WritePin(GPIOB, GPIO_PIN_9, GPIO_PIN_SET);
    HAL_Delay(5);
}

void w25q64_write_enable(){
    uint8_t cmd[1] = {0x06};
    spi_steami_transfer_receive(cmd, cmd, 1, 1000);
}

uint8_t w25q64_read_device_id(){
    uint8_t buffer[5] = {0xAB, 0x00, 0x00, 0x00, 0x00};
    
    spi_steami_transfer_receive(buffer, buffer, 5, 1000);

    return buffer[4];
}

uint8_t w25q64_read_manufacturer_id(){
    uint8_t buffer[6] = {0x90, 0x00, 0x00, 0x00, 0x00, 0x00};

    spi_steami_transfer_receive(buffer, buffer, 6, 1000);

    return buffer[4];
}

bool w25q64_is_busy(){
    uint8_t status = w25q64_read_status_register_1();
    return (status & 0x01) > 0;
}

bool w25q64_is_WEL(){
    uint8_t status = w25q64_read_status_register_1();
    return (status & 0x02) > 0;
}

uint8_t w25q64_read_status_register_1(){
    uint8_t status[2] = {0x05, 0x00};

    spi_steami_transfer_receive(status, status, 2, 1000);

    return status[1];
}

uint8_t w25q64_read_status_register_2(){
    uint8_t status[2] = {0x35, 0x00};

    spi_steami_transfer_receive(status, status, 2, 1000);

    return status[1];
}

uint8_t w25q64_read_status_register_3(){
    uint8_t status[2] = {0x15, 0x00};

    spi_steami_transfer_receive(status, status, 2, 1000);

    return status[1];
}

bool w25q64_read_data(uint8_t* data, uint32_t address, uint16_t read_len){
    uint16_t buffer_len = 4 + read_len;


    if( buffer_len > 260 ){ buffer_len = 260; }

    uint8_t buffer[buffer_len];
    memset(buffer, 0xFF, buffer_len);

    buffer[0] = 0x03;
    buffer[1] = (address & 0x00FF0000) >> 16;
    buffer[2] = (address & 0x0000FF00) >> 8;
    buffer[3] = address & 0x000000FF;

    if( spi_steami_transfer_receive(buffer, buffer, buffer_len, 1000) ){
        memcpy(data, buffer + 4, read_len);
        return true;
    }

    return false;
}

bool w25q64_page_program(uint8_t* data, uint32_t address, uint16_t data_len){
    uint16_t buffer_len = 4 + data_len;

    if( buffer_len > 260 ){ buffer_len = 260; }

    uint8_t buffer[buffer_len];
    memcpy(buffer+4, data, data_len);

    buffer[0] = 0x02;
    buffer[1] = (address & 0x00FF0000) >> 16;
    buffer[2] = (address & 0x0000FF00) >> 8;
    buffer[3] = address & 0x000000FF;

    return spi_steami_transfer(buffer, buffer_len, 1000);
}

bool w25q64_sector_erase(uint32_t address){
    uint8_t buffer[4];
    buffer[0] = 0x20;
    buffer[1] = (address & 0x00FF0000) >> 16;
    buffer[2] = (address & 0x0000FF00) >> 8;
    buffer[3] = address & 0x000000FF;

    return spi_steami_transfer(buffer, 4, 1000);
}

void w25q64_block_32k_erase(uint32_t address){
    uint8_t buffer[4] = { 0x52, (address & 0x00FF0000) >> 16, (address & 0x0000FF00) >> 8, address & 0x000000FF};

    spi_steami_transfer(buffer, 4, 1000);
}


void w25q64_block_64k_erase(uint32_t address){
    uint8_t buffer[4] = { 0xD8, (address & 0x00FF0000) >> 16, (address & 0x0000FF00) >> 8, address & 0x000000FF};

    spi_steami_transfer(buffer, 4, 1000);
}

void w25q64_chip_erase(){
    uint8_t cmd = 0xC7;
    spi_steami_transfer(&cmd, 1, 1000);
}