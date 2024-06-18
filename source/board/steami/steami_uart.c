#include "steami_uart.h"

#include <stdlib.h>

#include "stm32f1xx_hal_uart.h"
#include "stm32f1xx_hal_rcc.h"
#include "stm32f1xx_hal_gpio.h"
#include "stm32f1xx_hal_usart.h"

static UART_HandleTypeDef steami_uart;

static uint8_t tx_buffer[STEAMI_UART_TX_BUFFER] = {0};

static uint16_t tx_buffer_head = 0;
static uint16_t tx_buffer_tail = 0;
static uint16_t tx_buffer_size = 0;

void USART1_IRQHandler(){
    HAL_UART_IRQHandler(&steami_uart);
}

void HAL_UART_MspInit(UART_HandleTypeDef *huart){
    GPIO_InitTypeDef gpio;

    __HAL_RCC_GPIOB_CLK_ENABLE();
    __HAL_RCC_USART1_CLK_ENABLE();

    __HAL_AFIO_REMAP_USART1_ENABLE();

    //TX pin
    gpio.Pin = GPIO_PIN_6 | GPIO_PIN_7;
    gpio.Speed = GPIO_SPEED_FREQ_HIGH;
    gpio.Mode = GPIO_MODE_AF_PP;
    gpio.Pull = GPIO_PULLUP;
    HAL_GPIO_Init(GPIOB, &gpio);


    HAL_NVIC_SetPriority(USART1_IRQn, 3, 0);
    HAL_NVIC_EnableIRQ(USART1_IRQn);
}

void HAL_UART_MspDeInit(UART_HandleTypeDef *huart){
    HAL_NVIC_DisableIRQ(USART1_IRQn);
}

bool steami_uart_init(){
    steami_uart.Instance = USART1;
    steami_uart.Init.BaudRate = 115200;
    steami_uart.Init.WordLength = USART_WORDLENGTH_8B;
    steami_uart.Init.StopBits = USART_STOPBITS_1;
    steami_uart.Init.Parity = USART_PARITY_NONE;
    steami_uart.Init.Mode = USART_MODE_TX_RX;
    steami_uart.Init.HwFlowCtl = UART_HWCONTROL_NONE;
    steami_uart.Init.OverSampling = UART_OVERSAMPLING_16;

    return HAL_UART_Init(&steami_uart) == HAL_OK;
}

void steami_uart_write_data(uint8_t* data, uint16_t size){

    if( tx_buffer_head >= tx_buffer_tail ){
        uint16_t available_after = STEAMI_UART_TX_BUFFER - tx_buffer_head;
        uint16_t bytes_to_write = (available_after > size) ? size : available_after;

        memcpy(&tx_buffer[tx_buffer_head], data, bytes_to_write);
        tx_buffer_head += bytes_to_write;
        tx_buffer_size += bytes_to_write;

        if( bytes_to_write != size ){
            uint16_t remain_byte = size - bytes_to_write;
            uint16_t remain_bytes_to_write = (tx_buffer_tail > remain_byte) ? remain_byte : tx_buffer_tail;

            memcpy(&tx_buffer[0], &data[bytes_to_write], remain_bytes_to_write);
            tx_buffer_head += remain_bytes_to_write;
            tx_buffer_size += remain_bytes_to_write;
        }
    }
    else{
        uint16_t available = tx_buffer_tail - tx_buffer_head;
        uint16_t bytes_to_write = (size > available) ? available : size;
        memcpy(&tx_buffer[tx_buffer_head], data, bytes_to_write);
        tx_buffer_head += bytes_to_write;
        tx_buffer_size += bytes_to_write; 
    }

    if( tx_buffer_head >= STEAMI_UART_TX_BUFFER){
        tx_buffer_head -= STEAMI_UART_TX_BUFFER;
    }
}

void steami_uart_write_string(char* str){
    uint16_t size = strlen(str);
    steami_uart_write_data( (uint8_t*)str, size );
}

void steami_uart_write_number(uint32_t num, STEAMI_UART_BASE base){
    char result[12] = {0};

    switch (base)
    {
        case DEC:
            sprintf(result, "%u", num);
            break;

        case HEX:
            sprintf(result, "0x%08X", num);
            break;
        
        default:
            return;
    }

    steami_uart_write_string(result);
}

bool steami_uart_send(){
    if( tx_buffer_size == 0){
        return false;
    }

    HAL_StatusTypeDef status;

    if( tx_buffer_head > tx_buffer_tail ){
        status = HAL_UART_Transmit_IT(&steami_uart, &tx_buffer[tx_buffer_tail], tx_buffer_head - tx_buffer_tail);
        tx_buffer_size = 0;
        tx_buffer_tail = tx_buffer_head;
    }
    else{
        status = HAL_UART_Transmit_IT(&steami_uart, &tx_buffer[tx_buffer_tail], STEAMI_UART_TX_BUFFER - tx_buffer_tail);
        tx_buffer_size -= STEAMI_UART_TX_BUFFER - tx_buffer_tail;
        tx_buffer_tail = 0;
    }

    return status == HAL_OK;
}

uint16_t steami_uart_tx_size(){ return tx_buffer_size; }