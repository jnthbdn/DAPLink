#include "steami_spi.h"

#include <stdint.h>

#include "stm32f1xx_hal_gpio.h"

static SPI_HandleTypeDef hspi1;

void HAL_SPI_MspInit(SPI_HandleTypeDef *hspi){
    __HAL_RCC_SPI1_CLK_ENABLE();
    __HAL_RCC_GPIOB_CLK_ENABLE();
    __HAL_RCC_GPIOC_CLK_ENABLE();
    __HAL_RCC_AFIO_CLK_ENABLE();


    GPIO_InitTypeDef spi_gpio = {0};

    spi_gpio.Pin = GPIO_PIN_3 | GPIO_PIN_4 | GPIO_PIN_5;
    spi_gpio.Mode = GPIO_MODE_AF_PP;
    spi_gpio.Pull = GPIO_NOPULL;
    spi_gpio.Speed = GPIO_SPEED_FREQ_HIGH;

    __HAL_AFIO_REMAP_SPI1_ENABLE();
    HAL_GPIO_Init(GPIOB, &spi_gpio);


    spi_gpio.Pin = GPIO_PIN_15;
    spi_gpio.Mode = GPIO_MODE_OUTPUT_PP;
    spi_gpio.Pull = GPIO_NOPULL;
    HAL_GPIO_Init(GPIOC, &spi_gpio);
    HAL_GPIO_WritePin(GPIOC, GPIO_PIN_15, GPIO_PIN_SET);
}

void HAL_SPI_MspDeInit(SPI_HandleTypeDef *hspi){
    __HAL_RCC_SPI1_CLK_DISABLE();
}

uint32_t get_baudrate_prescaler(uint32_t target_baudrate){
    uint32_t freq = HAL_RCC_GetPCLK2Freq();

    if( target_baudrate >= freq / 2 ){
        return SPI_BAUDRATEPRESCALER_2;
    }
    else if( target_baudrate >= freq / 4){
        return SPI_BAUDRATEPRESCALER_4;
    }
    else if( target_baudrate >= freq / 8){
        return SPI_BAUDRATEPRESCALER_8;
    }
    else if( target_baudrate >= freq / 16){
        return SPI_BAUDRATEPRESCALER_16;
    }
    else if( target_baudrate >= freq / 32){
        return SPI_BAUDRATEPRESCALER_32;
    }
    else if( target_baudrate >= freq / 64){
        return SPI_BAUDRATEPRESCALER_64;
    }
    else if( target_baudrate >= freq / 128){
        return SPI_BAUDRATEPRESCALER_128;
    }
    else{
        return SPI_BAUDRATEPRESCALER_256;
    }
}

void enable_cs(){
    HAL_GPIO_WritePin(GPIOC, GPIO_PIN_15, GPIO_PIN_RESET);
}

void disable_cs(){
    HAL_GPIO_WritePin(GPIOC, GPIO_PIN_15, GPIO_PIN_SET);
}

bool steami_spi_init(){
    hspi1.Instance = SPI1;
    hspi1.Init.Mode = SPI_MODE_MASTER;
    hspi1.Init.Direction = SPI_DIRECTION_2LINES;
    hspi1.Init.DataSize = SPI_DATASIZE_8BIT;
    hspi1.Init.CLKPolarity = SPI_POLARITY_LOW;
    hspi1.Init.CLKPhase = SPI_PHASE_1EDGE;
    hspi1.Init.NSS = SPI_NSS_SOFT;
    hspi1.Init.BaudRatePrescaler = get_baudrate_prescaler(2000000);
    hspi1.Init.FirstBit = SPI_FIRSTBIT_MSB;
    hspi1.Init.TIMode = SPI_TIMODE_DISABLE;
    hspi1.Init.CRCCalculation = SPI_CRCCALCULATION_DISABLE;

    return HAL_SPI_Init(&hspi1) == HAL_OK;
}

bool spi_steami_transfer_receive(uint8_t* tx_buffer, uint8_t* rx_buffer, uint16_t buffer_size, uint32_t timeout){
    HAL_StatusTypeDef status;

    enable_cs();
    status = HAL_SPI_TransmitReceive(&hspi1, tx_buffer, rx_buffer, buffer_size, timeout);
    disable_cs();

    return status == HAL_OK;
}


bool spi_steami_transfer(uint8_t* data, uint16_t data_size, uint32_t timeout){
    HAL_StatusTypeDef status;

    enable_cs();
    status = HAL_SPI_Transmit(&hspi1, data, data_size, timeout );
    disable_cs();

    return status == HAL_OK;
}