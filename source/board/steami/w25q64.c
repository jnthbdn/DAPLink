#include "w25q64.h"
#include "steami_spi.h"

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

uint8_t w25q64_read_status(){
    uint8_t buffer[2] = {0x35, 0x00};

    spi_steami_transfer_receive(buffer, buffer, 2, 1000);

    return buffer[1];
}

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

    return w25q64_read_device_id() == 0x16;
}