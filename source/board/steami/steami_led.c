#include "steami_led.h"

#include "stm32f1xx.h"
#include "stm32f1xx_hal_gpio.h"
#include "stm32f1xx_hal_rcc.h"

void steami_led_init(){

    __GPIOA_CLK_ENABLE();
    __GPIOB_CLK_ENABLE();

    GPIO_InitTypeDef led = {0};
    led.Pin = GPIO_PIN_6;
    led.Mode = GPIO_MODE_OUTPUT_PP;
    led.Pull = GPIO_NOPULL;
    led.Speed = GPIO_SPEED_FREQ_HIGH;

    HAL_GPIO_Init(GPIOA, &led);

    led.Pin = GPIO_PIN_1;
    HAL_GPIO_Init(GPIOB, &led);

    steami_led_turn_off_green();
    steami_led_turn_off_blue();
}

void steami_led_toggle_green(){
    HAL_GPIO_TogglePin(GPIOB, GPIO_PIN_1);
}

void steami_led_turn_on_green(){
    HAL_GPIO_WritePin(GPIOB, GPIO_PIN_1, GPIO_PIN_RESET);
}

void steami_led_turn_off_green(){
    HAL_GPIO_WritePin(GPIOB, GPIO_PIN_1, GPIO_PIN_SET);
}

void steami_led_toggle_blue(){
    HAL_GPIO_TogglePin(GPIOA, GPIO_PIN_6);
}

void steami_led_turn_on_blue(){
    HAL_GPIO_WritePin(GPIOA, GPIO_PIN_6, GPIO_PIN_RESET);
}

void steami_led_turn_off_blue(){
    HAL_GPIO_WritePin(GPIOA, GPIO_PIN_6, GPIO_PIN_SET);
}