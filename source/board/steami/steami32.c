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

#include "steami_i2c.h"

char* plop_file = "Je suis super content d'être ici !";
uint32_t plop_file_size = 35;

static uint32_t read_plop_file(uint32_t sector_offset, uint8_t* data, uint32_t num_sector){

    uint32_t read_pos = VFS_SECTOR_SIZE * sector_offset;

    if( read_pos < plop_file_size ){
        for( uint32_t i = 0; (read_pos+ i)  < plop_file_size; ++i ){
            data[i] = plop_file[read_pos + i];
        }
    }

    return VFS_SECTOR_SIZE;
}

void vfs_user_build_filesystem_hook(){
    vfs_create_file("PLOP    TXT", read_plop_file, 0, plop_file_size);
}

static void prerun_board_config(void)
{
    steami_i2c_init();
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
