#include "steami_spi.h"

#include "stm32f1xx_hal_gpio.h"
#include "stdint.h"

#include "steami_i2c.h"

SPI_HandleTypeDef* handle_spi = NULL;

void HAL_SPI_MspInit(SPI_HandleTypeDef *hspi){
    __HAL_RCC_SPI1_CLK_ENABLE();
    __HAL_RCC_GPIOB_CLK_ENABLE();
    __HAL_RCC_GPIOC_CLK_ENABLE();


    GPIO_InitTypeDef spi_gpio = {0};

    spi_gpio.Pin = GPIO_PIN_3 | GPIO_PIN_4 | GPIO_PIN_5;
    spi_gpio.Mode = GPIO_MODE_AF_PP;
    spi_gpio.Pull = GPIO_PULLUP;
    spi_gpio.Speed = GPIO_SPEED_FREQ_HIGH;

    __HAL_AFIO_REMAP_SPI1_ENABLE();
    HAL_GPIO_Init(GPIOB, &spi_gpio);


    spi_gpio.Pin = GPIO_PIN_15;
    spi_gpio.Mode = GPIO_MODE_OUTPUT_PP;
    HAL_GPIO_Init(GPIOC, &spi_gpio);
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
    /*
        Erratum 2.3.7 "I2C1 with SPI1 remapped and used in master mode" https://www.st.com/content/ccc/resource/technical/document/errata_sheet/7d/02/75/64/17/fc/4d/fd/CD00190234.pdf/files/CD00190234.pdf/jcr:content/translations/en.CD00190234.pdf
        
        **Descirption**
        When the following conditions are met:
            • I2C1 and SPI1 are clocked.
            • SPI1 is remapped.
            • I/O port pin PB5 is configured as an alternate function output
        there is a conflict between the SPI1 MOSI and the I2C1 SMBA signals (even if SMBA is not used).
        
        **Workaround**
            Do not use SPI1 remapped in master mode and I2C1 together.
            When using SPI1 remapped, the I2C1 clock must be disabled
    */
    steami_i2c_deinit();
    steami_spi_init();
    HAL_GPIO_WritePin(GPIOC, GPIO_PIN_15, GPIO_PIN_RESET);
    HAL_Delay(1);
}

void disable_cs(){
    HAL_GPIO_WritePin(GPIOC, GPIO_PIN_15, GPIO_PIN_SET);
    steami_spi_deinit();
    steami_i2c_init();
}

void steami_spi_set_handler(SPI_HandleTypeDef* hspi){
    handle_spi = hspi;
}

HAL_StatusTypeDef steami_spi_init(){

    if( handle_spi == NULL ) return HAL_ERROR;

    handle_spi->Instance = SPI1;
    handle_spi->Init.Mode = SPI_MODE_MASTER;
    handle_spi->Init.Direction = SPI_DIRECTION_2LINES;
    handle_spi->Init.DataSize = SPI_DATASIZE_8BIT;
    handle_spi->Init.CLKPolarity = SPI_POLARITY_LOW;
    handle_spi->Init.CLKPhase = SPI_PHASE_1EDGE;
    handle_spi->Init.NSS = SPI_NSS_SOFT;
    handle_spi->Init.BaudRatePrescaler = get_baudrate_prescaler(2000000);
    handle_spi->Init.FirstBit = SPI_FIRSTBIT_MSB;
    handle_spi->Init.TIMode = SPI_TIMODE_DISABLE;
    handle_spi->Init.CRCCalculation = SPI_CRCCALCULATION_DISABLE;

    return HAL_SPI_Init(handle_spi);
}

HAL_StatusTypeDef steami_spi_deinit(){
    if( handle_spi == NULL ) return HAL_ERROR;

    return HAL_SPI_DeInit(handle_spi);
}


HAL_StatusTypeDef spi_steami_transfer_receive(uint8_t* tx_buffer, uint8_t* rx_buffer, uint16_t buffer_size, uint32_t timeout){
    if( handle_spi == NULL ) return HAL_ERROR;

    HAL_StatusTypeDef status;

    enable_cs();
    status = HAL_SPI_TransmitReceive(handle_spi, tx_buffer, rx_buffer, buffer_size, timeout);
    disable_cs();

    return status;
}


HAL_StatusTypeDef spi_steami_transfer(uint8_t* data, uint16_t data_size, uint32_t timeout){
    if( handle_spi == NULL ) return HAL_ERROR;

    HAL_StatusTypeDef status;

    enable_cs();
    status = HAL_SPI_Transmit(handle_spi, data, data_size, timeout );
    disable_cs();

    return status;
}