#include "steami_i2c.h"

#include "stm32f1xx_hal.h"
#include "stm32f1xx_hal_rcc.h"
#include "stm32f1xx_hal_gpio.h"

#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <stdbool.h>

#include "steami_uart.h"
#include "steami_i2c_dma.h"

static uint32_t time_master_ask_data = 0;
static bool is_master_wait_data = false;
static bool is_listen_I2C = false;
static I2C_HandleTypeDef hi2c2;
static steami_cmd_callback on_cmd_recv = NULL;

static uint8_t rx_i2c_command = 0x00;
static uint8_t rx_i2c_argument[STEAMI_I2C_RX_BUFFER_SIZE] = {0};
static uint8_t rx_i2c_len_argument = 0;

static uint8_t tx_i2c_data[STEAMI_I2C_TX_BUFFER_SIZE] = {0};
static uint16_t tx_i2c_len_data = 0;


static bool is_command_valid(uint8_t cmd){
    switch (cmd)
    {
        case WHO_AM_I:
        case CLEAR_FLASH:
        case SET_FILENAME:
        case GET_FILENAME:
        case WRITE_DATA:
        case READ_SECTOR:
        case STATUS:
        case ERROR_STATUS:
            return true;
        
        default:
            return false;
    }
}

void I2C2_EV_IRQHandler(void){
    HAL_I2C_EV_IRQHandler(&hi2c2);
}

void I2C2_ER_IRQHandler(void){
    HAL_I2C_ER_IRQHandler(&hi2c2);
}

void HAL_I2C_MspInit(I2C_HandleTypeDef *hi2c){
    __HAL_RCC_I2C2_CLK_ENABLE();
    __HAL_RCC_GPIOB_CLK_ENABLE();
    __HAL_RCC_AFIO_CLK_ENABLE();

    GPIO_InitTypeDef i2c_gpio = {0};

    i2c_gpio.Pin = GPIO_PIN_10 | GPIO_PIN_11;
    i2c_gpio.Mode = GPIO_MODE_AF_OD;
    i2c_gpio.Pull = GPIO_NOPULL;
    i2c_gpio.Speed = GPIO_SPEED_FREQ_HIGH;

    HAL_GPIO_Init(GPIOB, &i2c_gpio);

    HAL_NVIC_SetPriority(I2C2_EV_IRQn, 0, 0);
    HAL_NVIC_SetPriority(I2C2_ER_IRQn, 0, 0);
    HAL_NVIC_EnableIRQ(I2C2_EV_IRQn);
    HAL_NVIC_EnableIRQ(I2C2_ER_IRQn);
}

void HAL_I2C_MspDeInit(I2C_HandleTypeDef *hi2c){
    (void)hi2c;

    HAL_NVIC_DisableIRQ(I2C2_EV_IRQn);
    HAL_NVIC_DisableIRQ(I2C2_ER_IRQn);
    __HAL_RCC_I2C2_CLK_DISABLE();
}

void HAL_I2C_AddrCallback(I2C_HandleTypeDef *hi2c, uint8_t TransferDirection, uint16_t AddrMatchCode)
{
    if(TransferDirection == I2C_DIRECTION_TRANSMIT) 
	{
        tx_i2c_len_data = 0;
        rx_i2c_len_argument = 0;
        HAL_I2C_Slave_Seq_Receive_IT(hi2c, &rx_i2c_command, 1, I2C_NEXT_FRAME);
    }
	else if(TransferDirection == I2C_DIRECTION_RECEIVE)
	{
        is_master_wait_data = true;
        time_master_ask_data = HAL_GetTick();
	}
}

void HAL_I2C_SlaveRxCpltCallback(I2C_HandleTypeDef *hi2c)
{
    HAL_I2C_Slave_Seq_Receive_IT(hi2c, rx_i2c_argument + rx_i2c_len_argument, 1, I2C_NEXT_FRAME);
    rx_i2c_len_argument++;

    if( rx_i2c_len_argument >= STEAMI_I2C_RX_BUFFER_SIZE){
        rx_i2c_len_argument = 0;
    }
}

void HAL_I2C_SlaveTxCpltCallback(I2C_HandleTypeDef *hi2c){
    if( is_listen_I2C ) HAL_I2C_EnableListen_IT(hi2c);
}

void HAL_I2C_ListenCpltCallback(I2C_HandleTypeDef *hi2c){

    // __disable_irq();

    if( is_command_valid(rx_i2c_command) && on_cmd_recv != NULL ){
        on_cmd_recv( (steami_i2c_command)rx_i2c_command, rx_i2c_argument, rx_i2c_len_argument - 1);
        rx_i2c_command = 0;
    }

    if( is_listen_I2C ) HAL_I2C_EnableListen_IT(hi2c);

    // __enable_irq();
}

void HAL_I2C_ErrorCallback(I2C_HandleTypeDef *hi2c){
    steami_uart_write_string("[I2C ERROR] ");
    steami_uart_write_number(hi2c->ErrorCode, HEX);

    switch( hi2c->ErrorCode ){

        case HAL_I2C_ERROR_NONE     :
            steami_uart_write_string(" (HAL_I2C_ERROR_NONE)\n");
            break;

        case HAL_I2C_ERROR_BERR     :
            steami_uart_write_string(" (HAL_I2C_ERROR_BERR)\n");
            steami_uart_write_string("Re-init I2C...\n");

            if( is_master_wait_data ){
                tx_i2c_data[0] = 0xFF;
                HAL_I2C_Slave_Seq_Transmit_IT(&hi2c2, tx_i2c_data, 1, I2C_LAST_FRAME);
                is_master_wait_data = false;
            }

            steami_i2c_deinit();
            steami_i2c_init();
            break;

        case HAL_I2C_ERROR_ARLO     :
            steami_uart_write_string(" (HAL_I2C_ERROR_ARLO)\n");
            break;

        case HAL_I2C_ERROR_AF       :
            steami_uart_write_string(" (HAL_I2C_ERROR_AF)\n");
            break;

        case HAL_I2C_ERROR_OVR      :
            steami_uart_write_string(" (HAL_I2C_ERROR_OVR)\n");
            break;

        case HAL_I2C_ERROR_DMA      :
            steami_uart_write_string(" (HAL_I2C_ERROR_DMA)\n");
            break;

        case HAL_I2C_ERROR_TIMEOUT  :
            steami_uart_write_string(" (HAL_I2C_ERROR_TIMEOUT)\n");
            break;

        case HAL_I2C_ERROR_SIZE     :
            steami_uart_write_string(" (HAL_I2C_ERROR_SIZE)\n");
            break;

        case HAL_I2C_ERROR_DMA_PARAM:
            steami_uart_write_string(" (HAL_I2C_ERROR_DMA_PARAM)\n");
            break;

        case HAL_I2C_WRONG_START    :
            steami_uart_write_string(" (HAL_I2C_WRONG_START)\n");
            break;

        default:
            steami_uart_write_string(" (UNKNOWN)\n");
            steami_i2c_deinit();
            steami_i2c_init();
            break;
    }

}
void HAL_I2C_AbortCpltCallback(I2C_HandleTypeDef *hi2c){
    steami_uart_write_string("[I2C ABORT]\n");
}

bool steami_i2c_init(){
    hi2c2.Instance = I2C2;
    hi2c2.Init.ClockSpeed = 100000;
    hi2c2.Init.OwnAddress1 = I2C_STEAMI_ADDRESS;
    hi2c2.Init.OwnAddress2 = 0x00;
    hi2c2.Init.DutyCycle = I2C_DUTYCYCLE_2;
    hi2c2.Init.AddressingMode = I2C_ADDRESSINGMODE_7BIT;
    hi2c2.Init.DualAddressMode = I2C_DUALADDRESS_DISABLE;
    hi2c2.Init.GeneralCallMode = I2C_GENERALCALL_DISABLED;
    hi2c2.Init.NoStretchMode = I2C_NOSTRETCH_DISABLE;

    if( HAL_I2C_Init(&hi2c2) != HAL_OK ){
        return false;
    }

    if( HAL_I2C_EnableListen_IT(&hi2c2) != HAL_OK ){
        return false;
    }

    is_listen_I2C = true;
    return steami_i2c_dma_init(&hi2c2);
}

void steami_i2c_deinit(){
    is_listen_I2C = false;
    HAL_I2C_DisableListen_IT(&hi2c2);
    HAL_I2C_DeInit(&hi2c2);
}

void steami_i2c_on_receive_command( steami_cmd_callback callback ){
    on_cmd_recv = callback;
}

void steami_i2c_set_tx_data(uint8_t* data, uint16_t len){
    tx_i2c_len_data = (len > STEAMI_I2C_TX_BUFFER_SIZE) ? STEAMI_I2C_TX_BUFFER_SIZE : len;
    memcpy(tx_i2c_data, data, tx_i2c_len_data);
}

void steami_i2c_process(){
    if( is_listen_I2C && READ_BIT(hi2c2.Instance->CR1, I2C_CR1_PE) == 0){
        steami_i2c_init();
        return;
    }

    if( is_master_wait_data && (HAL_GetTick() - time_master_ask_data) >= I2C_STEAMI_MASTER_READ_TIMEOUT_MS){

        tx_i2c_data[0] = 0xFF;
        HAL_I2C_Slave_Seq_Transmit_IT(&hi2c2, tx_i2c_data, 1, I2C_LAST_FRAME);

        is_master_wait_data = false;
        tx_i2c_len_data = 0;
        steami_uart_write_string("[I2C] Send data timeout...\n");
    }
    else if( is_master_wait_data && tx_i2c_len_data > 0 ){

        HAL_I2C_DisableListen_IT(&hi2c2);
        switch( HAL_I2C_Slave_Transmit_DMA(&hi2c2, tx_i2c_data, tx_i2c_len_data) ){
            case HAL_OK:
                // steami_uart_write_string("I2C DMA Success\n");
                break;

            case HAL_ERROR:
                steami_uart_write_string("I2C DMA ERROR\n");
                break;

            case HAL_BUSY:
                steami_uart_write_string("I2C DMA BUSY\n");
                break;

            case HAL_TIMEOUT:
                steami_uart_write_string("I2C DMA TIMEOUT\n");
                break;


            default:
                steami_uart_write_string("I2C DMA DEFAULT ??\n");
                break;
        }

        is_master_wait_data = false;
        tx_i2c_len_data = 0;
    }
}