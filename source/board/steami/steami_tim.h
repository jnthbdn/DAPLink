// #pragma once

// #include <stdbool.h>
// #include <stdint.h>

// typedef void(*steami_tim_callback)(void);

// /**
//  * @brief Initialize TIM3
//  * 
//  * @param freq The frequency (in Hertz) at which the IRQ will be produced. Range: [1; 32767]. 
//  * @return TRUE if successful, FALSE otherwise
//  */
// bool steami_tim_init(uint32_t freq);

// /**
//  * @brief Set the function called on Timer IRQ
//  * 
//  * @param cb the callback function (or NULL to disable)
//  */
// void steami_tim_set_callback( steami_tim_callback cb );