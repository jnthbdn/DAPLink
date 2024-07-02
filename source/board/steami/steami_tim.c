// #include "steami_tim.h"


// #include "stm32f1xx.h"
// #include "stm32f1xx_hal_tim.h"
// #include "stm32f1xx_hal_rcc.h"

// #define  FREQ_TARGET_TIM3    (uint32_t)100000

// static TIM_HandleTypeDef timer3;
// static steami_tim_callback irq_callback = NULL;

// void TIM3_IRQHandler(void)
// {
//     HAL_TIM_IRQHandler(&timer3);
// }

// void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim){
//     if( htim == &timer3 && irq_callback != NULL){
//         irq_callback();
//     }
// }

// void HAL_TIM_Base_MspInit(TIM_HandleTypeDef *htim){

//     __HAL_RCC_TIM3_CLK_ENABLE();

//     HAL_NVIC_SetPriority(TIM3_IRQn, 3, 0);
//     HAL_NVIC_EnableIRQ(TIM3_IRQn);
// }

// uint32_t tim3_clk_div(uint32_t apb1clkdiv)
// {
//     switch (apb1clkdiv) {
//         case RCC_CFGR_PPRE1_DIV2:
//             return 1;
//         case RCC_CFGR_PPRE1_DIV4:
//             return 2;
//         case RCC_CFGR_PPRE1_DIV8:
//             return 4;
//         case RCC_CFGR_PPRE1_DIV16:
//             return 8;
//         default:
//             return 1;
//     }
// }

// bool steami_tim_init(uint32_t freq){

//     if( freq < 1 || freq > 32767 ){
//         return false;
//     }

//     uint32_t unused;
//     RCC_ClkInitTypeDef clk_init;

//     HAL_RCC_GetClockConfig(&clk_init, &unused);

//     uint32_t period_div = (10 * freq) / 5;

//     uint32_t source_clock = SystemCoreClock / tim3_clk_div(clk_init.AHBCLKDivider);
//     uint32_t prescaler = (uint32_t)(source_clock / FREQ_TARGET_TIM3) - 1;
//     uint32_t period = FREQ_TARGET_TIM3 / period_div;

//     timer3.Instance = TIM3;

//     timer3.Init.Period = period;
//     timer3.Init.Prescaler = prescaler;
//     timer3.Init.ClockDivision = 0;
//     timer3.Init.CounterMode = TIM_COUNTERMODE_UP;
//     timer3.Init.RepetitionCounter = 0;

//     HAL_TIM_Base_DeInit(&timer3);
    
//     if( HAL_TIM_Base_Init(&timer3) != HAL_OK ){
//         return false;
//     }

//     return HAL_TIM_Base_Start_IT(&timer3) == HAL_OK;
// }


// void steami_tim_set_callback( steami_tim_callback cb ){
//     irq_callback = cb;
// }