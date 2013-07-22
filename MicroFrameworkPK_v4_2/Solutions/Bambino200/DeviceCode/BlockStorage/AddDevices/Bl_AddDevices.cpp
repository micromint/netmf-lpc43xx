////////////////////////////////////////////////////////////////////////////////
// Bl_AddDevices.cpp - BlockStorage device functions for Micromint Bambino 200
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
//
// Copyright (c) Microsoft Corporation.  All rights reserved.
// Ported to Bambino 200 by Micromint USA <support@micromint.com>
////////////////////////////////////////////////////////////////////////////////

#include <tinyhal.h>

extern struct BlockStorageDevice  g_LPC43XX_BS;
extern struct IBlockStorageDevice g_LPC43XX_SPIFI_DeviceTable;
extern struct BLOCK_CONFIG        g_LPC43XX_BS_Config;

void BlockStorage_AddDevices() {
    BlockStorageList::AddDevice( &g_LPC43XX_BS,
                                 &g_LPC43XX_SPIFI_DeviceTable,
                                 &g_LPC43XX_BS_Config, FALSE );
}
