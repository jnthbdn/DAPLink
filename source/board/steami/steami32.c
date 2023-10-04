/**
 * @file    steamy32.c
 * @brief   board ID for the STeamy32 board
 *
 * DAPLink Interface Firmware
 * Copyright (c) 2009-2019, ARM Limited, All Rights Reserved
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

#include "target_family.h"
#include "target_board.h"

#include "error_steami_file.h"
#include "steami_i2c.h"
#include "steami_spi.h"
#include "steami_flash.h"

SPI_HandleTypeDef hspi = {0};
I2C_HandleTypeDef hi2c = {0};


char plop[50] = {0};

void vfs_user_build_filesystem_hook(){
    create_steami_error_file("STMI_ERRTXT");
}

static void prerun_board_config(void)
{
    steami_i2c_set_handler(&hi2c);
    steami_spi_set_handler(&hspi);

    // Test I2C init
    if( steami_i2c_init() != HAL_OK ){
        set_steami_error_content("I2C init failed !");
        return;
    }

    // Test SPI init
    if( steami_spi_init() != HAL_OK ){
        set_steami_error_content("SPI init failed !");
        return;
    }

    if( ! steami_flash_init() ){
        set_steami_error_content( "Flash init failed !");
        return;
    }

    sprintf(plop, "Flash\r\n\tID: 0x%02X\r\n\tManufacturer: 0x%02X", read_device_id(), read_manufacturer_id());
    set_steami_error_content(plop);
}

const board_info_t g_board_info = {
    .info_version = kBoardInfoVersion,
    .board_id = "0839",
    .family_id = kStub_HWReset_FamilyID,
    .daplink_url_name   = "STEAMI  HTM",
    .daplink_drive_name = "STeaMi     ",
    .daplink_target_url = "https://makecode.lets-steam.eu/",
    .prerun_board_config = prerun_board_config,
    .target_cfg = &target_device,
};
