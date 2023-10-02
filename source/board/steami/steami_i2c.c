#include "steami_i2c.h"

#include "stm32f1xx_hal.h"
#include "stm32f1xx_hal_dma.h"
#include "stm32f1xx_hal_i2c.h"
#include "stm32f1xx_hal_rcc.h"
#include "stm32f1xx_hal_gpio.h"

#include "stdint.h"

#define I2C_BUFFER_SIZE 1024

I2C_HandleTypeDef i2c_handle = {0};
uint8_t i2c_buffer[I2C_BUFFER_SIZE] = {0};

void I2C1_EV_IRQHandler(void){
    HAL_I2C_EV_IRQHandler(&i2c_handle);
}

void I2C1_ER_IRQHandler(void){
    HAL_I2C_ER_IRQHandler(&i2c_handle);
}


void HAL_I2C_MspInit(I2C_HandleTypeDef *hi2c){
    __HAL_RCC_I2C1_CLK_ENABLE();
    __HAL_RCC_GPIOB_CLK_ENABLE();

    GPIO_InitTypeDef i2c_gpio = {0};

    i2c_gpio.Pin = GPIO_PIN_6 | GPIO_PIN_7;
    i2c_gpio.Mode = GPIO_MODE_AF_OD;
    i2c_gpio.Pull = GPIO_NOPULL;
    i2c_gpio.Speed = GPIO_SPEED_FREQ_HIGH;

    __HAL_AFIO_REMAP_I2C1_DISABLE();
    HAL_GPIO_Init(GPIOB, &i2c_gpio);


    HAL_NVIC_SetPriority(I2C1_EV_IRQn, 1, 0);
    HAL_NVIC_SetPriority(I2C1_ER_IRQn, 1, 0);
    HAL_NVIC_EnableIRQ(I2C1_EV_IRQn);
    HAL_NVIC_EnableIRQ(I2C1_ER_IRQn);
}

void HAL_I2C_AddrCallback(I2C_HandleTypeDef *hi2c, uint8_t TransferDirection, uint16_t AddrMatchCode)
{
	if(TransferDirection == I2C_DIRECTION_TRANSMIT) 
	{
        HAL_I2C_Slave_Seq_Receive_IT(hi2c, i2c_buffer, 1, I2C_FIRST_AND_LAST_FRAME);
	}
	else
	{
        HAL_I2C_Slave_Seq_Transmit_IT(hi2c, i2c_buffer, 1, I2C_FIRST_AND_LAST_FRAME);
	}
}

void HAL_I2C_ListenCpltCallback(I2C_HandleTypeDef *hi2c){
    HAL_I2C_EnableListen_IT(hi2c);
}

void HAL_I2C_SlaveRxCpltCallback(I2C_HandleTypeDef *hi2c){
    HAL_GPIO_TogglePin(GPIOA, GPIO_PIN_6);
}

void HAL_I2C_ErrorCallback(I2C_HandleTypeDef *hi2c){
	HAL_I2C_EnableListen_IT(hi2c);
}

HAL_StatusTypeDef steami_i2c_init(){
    i2c_handle.Instance = I2C1;
    i2c_handle.Init.ClockSpeed = 100000;
    i2c_handle.Init.OwnAddress1 = 0x76;
    i2c_handle.Init.OwnAddress2 = 0x00;
    i2c_handle.Init.DutyCycle = I2C_DUTYCYCLE_2;
    i2c_handle.Init.AddressingMode = I2C_ADDRESSINGMODE_7BIT;
    i2c_handle.Init.DualAddressMode = I2C_DUALADDRESS_DISABLE;
    i2c_handle.Init.GeneralCallMode = I2C_GENERALCALL_DISABLED;
    i2c_handle.Init.NoStretchMode = I2C_NOSTRETCH_DISABLE;

    GPIO_InitTypeDef led = {0};
    led.Pin = GPIO_PIN_6;
    led.Mode = GPIO_MODE_OUTPUT_PP;
    led.Pull = GPIO_NOPULL;
    led.Speed = GPIO_SPEED_FREQ_LOW;

    HAL_GPIO_Init(GPIOA, &led);
    HAL_GPIO_WritePin(GPIOA, GPIO_PIN_6, GPIO_PIN_SET);

    return HAL_I2C_Init(&i2c_handle);
}

HAL_StatusTypeDef steami_i2c_start_slave(){
    return HAL_I2C_EnableListen_IT(&i2c_handle);
}