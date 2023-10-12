#include "steami_i2c.h"

#include "stm32f1xx_hal.h"
#include "stm32f1xx_hal_dma.h"
#include "stm32f1xx_hal_rcc.h"
#include "stm32f1xx_hal_gpio.h"

#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <stdbool.h>

#define STEAMI_I2C_BUFFER_SIZE 256
#define I2C_STEAMI_ADDRESS 0X76

typedef uint16_t (*steami_cmd_callback)(steami_i2c_command cmd, uint8_t* rx_data, uint16_t len_rx, uint8_t* tx_data);

static bool is_listen_I2C = false;
static I2C_HandleTypeDef* i2c_handle = NULL;
static steami_cmd_callback on_cmd_recv = NULL;

static uint8_t rx_i2c_command = 0x00;
static uint8_t* rx_i2c_argument = NULL;
static uint8_t rx_i2c_len_argument = 0;

static uint8_t* tx_i2c_data = NULL;
static uint8_t tx_i2c_len_data = 0;

static bool is_command_valid(uint8_t cmd){
    switch (cmd)
    {
        case WHO_AM_I:
        case CLEAR_FLASH:
        case SET_FILENAME:
        case GET_FILENAME:
        case WRITE_DATA:
            return true;
        
        default:
            return false;
    }
}

void I2C1_EV_IRQHandler(void){
    HAL_I2C_EV_IRQHandler(i2c_handle);
}

void I2C1_ER_IRQHandler(void){
    HAL_I2C_ER_IRQHandler(i2c_handle);
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

void HAL_I2C_MspDeInit(I2C_HandleTypeDef *hi2c){
    (void)hi2c;

    HAL_NVIC_DisableIRQ(I2C1_EV_IRQn);
    HAL_NVIC_DisableIRQ(I2C1_ER_IRQn);
    __HAL_RCC_I2C1_CLK_DISABLE();
}

void HAL_I2C_AddrCallback(I2C_HandleTypeDef *hi2c, uint8_t TransferDirection, uint16_t AddrMatchCode)
{
    if( AddrMatchCode != I2C_STEAMI_ADDRESS ) return;

	if(TransferDirection == I2C_DIRECTION_TRANSMIT) 
	{
        rx_i2c_len_argument = 0;
        HAL_I2C_Slave_Seq_Receive_IT(hi2c, &rx_i2c_command, 1, I2C_FIRST_FRAME);
	}
	else
	{
        if( tx_i2c_len_data == 0 ){
            tx_i2c_data[0] = 0x00;
            tx_i2c_len_data = 1;
        }

        // HAL_I2C_Slave_Seq_Transmit_IT(hi2c, tx_i2c_data, tx_i2c_len_data, I2C_FIRST_AND_LAST_FRAME);
        HAL_I2C_Slave_Transmit(hi2c, tx_i2c_data, tx_i2c_len_data, 1000);
        tx_i2c_len_data = 0;
	}
}

void HAL_I2C_SlaveRxCpltCallback(I2C_HandleTypeDef *hi2c)
{
    HAL_GPIO_TogglePin(GPIOA, GPIO_PIN_6);
    HAL_I2C_Slave_Seq_Receive_IT(hi2c, &rx_i2c_argument[rx_i2c_len_argument], 1, I2C_FIRST_FRAME);
    rx_i2c_len_argument++;
}

void HAL_I2C_ListenCpltCallback(I2C_HandleTypeDef *hi2c){

    if( is_command_valid(rx_i2c_command) && on_cmd_recv != NULL ){
        tx_i2c_len_data = on_cmd_recv( (steami_i2c_command)rx_i2c_command, rx_i2c_argument, rx_i2c_len_argument, tx_i2c_data);
    }

    if( is_listen_I2C ) HAL_I2C_EnableListen_IT(hi2c);
}

void HAL_I2C_ErrorCallback(I2C_HandleTypeDef *hi2c){
    if( is_listen_I2C ) HAL_I2C_EnableListen_IT(hi2c);
}

void steami_i2c_set_handler(I2C_HandleTypeDef *hi2c){
    i2c_handle = hi2c;
}

HAL_StatusTypeDef steami_i2c_init(){
    rx_i2c_argument = (uint8_t*)malloc(STEAMI_I2C_BUFFER_SIZE * sizeof(uint8_t));
    tx_i2c_data = (uint8_t*)malloc(STEAMI_I2C_MAX_TX_DATA * sizeof(uint8_t));

    if( i2c_handle == NULL || rx_i2c_argument == NULL || tx_i2c_data == NULL){
        return HAL_ERROR;
    }

    memset(rx_i2c_argument, '\0', STEAMI_I2C_BUFFER_SIZE);
    memset(tx_i2c_data, 0x00, STEAMI_I2C_MAX_TX_DATA);

    HAL_StatusTypeDef status;

    i2c_handle->Instance = I2C1;
    i2c_handle->Init.ClockSpeed = 100000;
    i2c_handle->Init.OwnAddress1 = I2C_STEAMI_ADDRESS;
    i2c_handle->Init.OwnAddress2 = 0x00;
    i2c_handle->Init.DutyCycle = I2C_DUTYCYCLE_2;
    i2c_handle->Init.AddressingMode = I2C_ADDRESSINGMODE_7BIT;
    i2c_handle->Init.DualAddressMode = I2C_DUALADDRESS_DISABLE;
    i2c_handle->Init.GeneralCallMode = I2C_GENERALCALL_DISABLED;
    i2c_handle->Init.NoStretchMode = I2C_NOSTRETCH_DISABLE;

    GPIO_InitTypeDef led = {0};
    led.Pin = GPIO_PIN_6;
    led.Mode = GPIO_MODE_OUTPUT_PP;
    led.Pull = GPIO_NOPULL;
    led.Speed = GPIO_SPEED_FREQ_LOW;

    HAL_GPIO_Init(GPIOA, &led);
    HAL_GPIO_WritePin(GPIOA, GPIO_PIN_6, GPIO_PIN_SET);

    status =  HAL_I2C_Init(i2c_handle);

    if( status != HAL_OK ){
        return status;
    }

    status =  HAL_I2C_EnableListen_IT(i2c_handle);

    if( status != HAL_OK ){
        return status;
    }

    is_listen_I2C = true;
    return HAL_OK;
}

HAL_StatusTypeDef steami_i2c_deinit(){
    if( i2c_handle == NULL ){
        return HAL_ERROR;
    }

    is_listen_I2C = false;
    HAL_I2C_DisableListen_IT(i2c_handle);
    return HAL_I2C_DeInit(i2c_handle);
}

void steami_i2c_on_receive_command( steami_cmd_callback callback ){
    on_cmd_recv = callback;
}