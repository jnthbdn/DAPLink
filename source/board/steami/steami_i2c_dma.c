#include "steami_i2c_dma.h"

#include "stm32f1xx_hal_def.h"
#include "stm32f1xx_hal_dma.h"
#include "stm32f1xx_hal_rcc.h"

static DMA_HandleTypeDef hdma4_i2c2_tx;
static DMA_HandleTypeDef hdma5_i2c2_rx;

void DMA1_Channel4_IRQHandler(){
    HAL_DMA_IRQHandler(&hdma4_i2c2_tx);
}

void DMA1_Channel5_IRQHandler(){
    HAL_DMA_IRQHandler(&hdma5_i2c2_rx);
}

bool steami_i2c_dma_init(I2C_HandleTypeDef * hi2c2){

    __HAL_RCC_DMA1_CLK_ENABLE();

    hdma4_i2c2_tx.Instance = DMA1_Channel4;
    hdma4_i2c2_tx.Init.Direction = DMA_MEMORY_TO_PERIPH;
    hdma4_i2c2_tx.Init.PeriphInc = DMA_PINC_DISABLE;
    hdma4_i2c2_tx.Init.MemInc = DMA_MINC_ENABLE;
    hdma4_i2c2_tx.Init.PeriphDataAlignment = DMA_PDATAALIGN_BYTE;
    hdma4_i2c2_tx.Init.MemDataAlignment = DMA_MDATAALIGN_BYTE;
    hdma4_i2c2_tx.Init.Mode = DMA_NORMAL;
    hdma4_i2c2_tx.Init.Priority = DMA_PRIORITY_LOW;

    if( HAL_DMA_Init(&hdma4_i2c2_tx) != HAL_OK ){
        return false;
    }


    hdma5_i2c2_rx.Instance = DMA1_Channel5;
    hdma5_i2c2_rx.Init.Direction = DMA_PERIPH_TO_MEMORY;
    hdma5_i2c2_rx.Init.PeriphInc = DMA_PINC_DISABLE;
    hdma5_i2c2_rx.Init.MemInc = DMA_MINC_ENABLE;
    hdma5_i2c2_rx.Init.PeriphDataAlignment = DMA_PDATAALIGN_BYTE;
    hdma5_i2c2_rx.Init.MemDataAlignment = DMA_MDATAALIGN_BYTE;
    hdma5_i2c2_rx.Init.Mode = DMA_CIRCULAR;
    hdma5_i2c2_rx.Init.Priority = DMA_PRIORITY_HIGH;

    if( HAL_DMA_Init(&hdma5_i2c2_rx) != HAL_OK ){
        return false;
    }

    __HAL_LINKDMA(hi2c2, hdmatx, hdma4_i2c2_tx);
    __HAL_LINKDMA(hi2c2, hdmarx, hdma5_i2c2_rx);


    HAL_NVIC_SetPriority(DMA1_Channel4_IRQn, 0, 0);
    HAL_NVIC_EnableIRQ(DMA1_Channel4_IRQn);

    HAL_NVIC_SetPriority(DMA1_Channel5_IRQn, 0, 0);
    HAL_NVIC_EnableIRQ(DMA1_Channel5_IRQn);

    return true;
}