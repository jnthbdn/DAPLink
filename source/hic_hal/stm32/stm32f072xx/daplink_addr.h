/**
 * @file    daplink_addr.h
 * @brief
 *
 * DAPLink Interface Firmware
 * Copyright (c) 2009-2018, ARM Limited, All Rights Reserved
 * SPDX-License-Identifier: Apache-2.0
 *
 * Licensed under the Apache License, Version 2.0 (the "License"); you may
 * not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS, WITHOUT
 * WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef DAPLINK_ADDR_H
#define DAPLINK_ADDR_H

#define DAPLINK_ROM_START               0x08000000
#if defined(INTERFACE_STM32F072XB)
#define DAPLINK_ROM_SIZE                0x00020000  // 128 KB
#elif defined(INTERFACE_STM32F072X8)
#define DAPLINK_ROM_SIZE                0x00010000  //  64 KB
#endif

#define DAPLINK_RAM_START               0x20000000
#define DAPLINK_RAM_SIZE                0x00004000  //  16 KB

/* ROM sizes */
/* stm32f072xx does not support DAPLink bootloader: use STDFU or SWD */
#define DAPLINK_ROM_BL_SIZE             0x00000000

#define DAPLINK_ROM_CONFIG_USER_SIZE    0x00000800
#define DAPLINK_ROM_IF_SIZE             (DAPLINK_ROM_SIZE - DAPLINK_ROM_CONFIG_USER_SIZE)

#define DAPLINK_ROM_BL_START            DAPLINK_ROM_START
#define DAPLINK_ROM_IF_START            DAPLINK_ROM_START
#define DAPLINK_ROM_CONFIG_USER_START   (DAPLINK_ROM_IF_START + DAPLINK_ROM_IF_SIZE)

/* RAM sizes */
#define DAPLINK_RAM_SHARED_SIZE         0x00000100
#define DAPLINK_RAM_APP_SIZE            (DAPLINK_RAM_SIZE - DAPLINK_RAM_SHARED_SIZE)

#define DAPLINK_RAM_APP_START           DAPLINK_RAM_START
#define DAPLINK_RAM_SHARED_START        (DAPLINK_RAM_APP_START + DAPLINK_RAM_APP_SIZE)

/* Flash Programming Info */
#define DAPLINK_SECTOR_SIZE             0x800
#define DAPLINK_MIN_WRITE_SIZE          4

/* Current build */
#if defined(DAPLINK_IF)

#define DAPLINK_ROM_APP_START           DAPLINK_ROM_IF_START
#define DAPLINK_ROM_APP_SIZE            DAPLINK_ROM_IF_SIZE
#define DAPLINK_ROM_UPDATE_START        0
#define DAPLINK_ROM_UPDATE_SIZE         0

#elif defined(DAPLINK_BL)

#error "stm32f072xx does not support DAPLink bootloader"

#else

#error "Build must be interface"

#endif

#endif
