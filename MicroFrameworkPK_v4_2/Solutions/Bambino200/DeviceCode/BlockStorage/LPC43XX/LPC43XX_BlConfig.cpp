////////////////////////////////////////////////////////////////////////////////
// LPC43XX_BlConfig.cpp - Block storage functions for Micromint Bambino 200
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

#define FLASH_MANUFACTURER_CODE              0x0000  // not used
#define FLASH_DEVICE_CODE                    0x0000  // not used
// Region 1: CLR                   -  6 * 64K blocks (384K)
#define FLASH_BASE_ADDRESS1                  0x14000000
#define FLASH_BLOCK_COUNT1                   6
#define FLASH_BYTES_PER_BLOCK1               (64*1024)
// Region 1: Config + slave core  -  4 * 16K blocks (64K)
#define FLASH_BASE_ADDRESS2                  0x14060000
#define FLASH_BLOCK_COUNT2                   4
#define FLASH_BYTES_PER_BLOCK2               (16*1024)
// Region 3: Deployment            - 25 * 64K blocks (1600K)
#define FLASH_BASE_ADDRESS3                  0x14070000
#define FLASH_BLOCK_COUNT3                   25
#define FLASH_BYTES_PER_BLOCK3               (64*1024)
// Region 4: Filesystem            - 32 * 64K blocks (ToDo)
#define FLASH_BYTES_PER_SECTOR               2
#define FLASH_BLOCK_ERASE_TYPICAL_TIME_USEC  1000000 // not used
#define FLASH_SECTOR_WRITE_TYPICAL_TIME_USEC 10      // not used
#define FLASH_BLOCK_ERASE_MAX_TIME_USEC      4000000 // not used
#define FLASH_SECTOR_WRITE_MAX_TIME_USEC     100     // not used
#define FLASH_BLOCK_ERASE_ACTUAL_TIME_USEC   10000   // not used

// EBIU Information

#define LPC43XX__CHIP_SELECT       0
#define LPC43XX__WAIT_STATES       5 // not used
#define LPC43XX__RELEASE_COUNTS    0
#define LPC43XX__BIT_WIDTH         16
#define LPC43XX__BASE_ADDRESS      0x14000000
#define LPC43XX__SIZE_IN_BYTES     0x00400000 // 4M flash
#define LPC43XX__WP_GPIO_PIN       GPIO_PIN_NONE
#define LPC43XX__WP_ACTIVE         FALSE

// BlockDeviceInformation

#define LPC43XX__IS_REMOVABLE      FALSE
#define LPC43XX__SUPPORTS_XIP      TRUE
#define LPC43XX__WRITE_PROTECTED   FALSE
#define LPC43XX__SUPP_COPY_BACK    FALSE
#define LPC43XX__NUM_REGIONS       3

#if defined(BUILD_RTM)
        #define MEMORY_BLOCKTYPE_SPECIAL  BlockRange::BLOCKTYPE_DEPLOYMENT
#else
        #define MEMORY_BLOCKTYPE_SPECIAL  BlockRange::BLOCKTYPE_FILESYSTEM 
#endif

const BlockRange g_LPC43XX_BlockRange1[] =
{
    { BlockRange::BLOCKTYPE_CODE      ,   0, 5 },  // 0x14000000 CLR         384k
};

const BlockRange g_LPC43XX_BlockRange2[] =
{
    { BlockRange::BLOCKTYPE_CONFIG    ,   0, 0 },  // 0x14060000 Config       16k
//    { BlockRange::BLOCKTYPE_CODE      ,   1, 3 },  // 0x14064000 Slave        48k
};

const BlockRange g_LPC43XX_BlockRange3[] =
{
    { BlockRange::BLOCKTYPE_DEPLOYMENT,   0, 24},  // 0x14070000 Deployment 1600k
};

const BlockRegionInfo  g_LPC43XX_BlkRegion[LPC43XX__NUM_REGIONS] = 
{
    {
        FLASH_BASE_ADDRESS1,    // ByteAddress   Start;           // Starting Sector address
        FLASH_BLOCK_COUNT1,     // UINT32        NumBlocks;       // total number of blocks in this region
        FLASH_BYTES_PER_BLOCK1, // UINT32        BytesPerBlock;   // Total number of bytes per block
        ARRAYSIZE_CONST_EXPR(g_LPC43XX_BlockRange1),
        g_LPC43XX_BlockRange1,
    },
    
    {
        FLASH_BASE_ADDRESS2,    // ByteAddress   Start;           // Starting Sector address
        FLASH_BLOCK_COUNT2,     // UINT32        NumBlocks;       // total number of blocks in this region
        FLASH_BYTES_PER_BLOCK2, // UINT32        BytesPerBlock;   // Total number of bytes per block
        ARRAYSIZE_CONST_EXPR(g_LPC43XX_BlockRange2),
        g_LPC43XX_BlockRange2,
    },
    
    {
        FLASH_BASE_ADDRESS3,    // ByteAddress   Start;           // Starting Sector address
        FLASH_BLOCK_COUNT3,     // UINT32        NumBlocks;       // total number of blocks in this region
        FLASH_BYTES_PER_BLOCK3, // UINT32        BytesPerBlock;   // Total number of bytes per block
        ARRAYSIZE_CONST_EXPR(g_LPC43XX_BlockRange3),
        g_LPC43XX_BlockRange3,
    }
};

const BlockDeviceInfo g_LPC43XX_DeviceInfo=
{
    {  
        LPC43XX__IS_REMOVABLE,             // BOOL Removable;
        LPC43XX__SUPPORTS_XIP,             // BOOL SupportsXIP;
        LPC43XX__WRITE_PROTECTED,          // BOOL WriteProtected;
        LPC43XX__SUPP_COPY_BACK            // BOOL SupportsCopyBack
    },
    FLASH_SECTOR_WRITE_TYPICAL_TIME_USEC,  // UINT32 MaxSectorWrite_uSec;
    FLASH_BLOCK_ERASE_ACTUAL_TIME_USEC,    // UINT32 MaxBlockErase_uSec;
    FLASH_BYTES_PER_SECTOR,                // UINT32 BytesPerSector;     

    FLASH_MEMORY_Size,                     // UINT32 Size;

    LPC43XX__NUM_REGIONS,                  // UINT32 NumRegions;
    g_LPC43XX_BlkRegion,                   // const BlockRegionInfo* pRegions;
};

struct MEMORY_MAPPED_NOR_BLOCK_CONFIG g_LPC43XX_BS_Config =
{
    { // BLOCK_CONFIG
        {
            LPC43XX__WP_GPIO_PIN,          // GPIO_PIN             Pin;
            LPC43XX__WP_ACTIVE,            // BOOL                 ActiveState;
        },

        &g_LPC43XX_DeviceInfo,             // BlockDeviceinfo
    },

    { // CPU_MEMORY_CONFIG
        LPC43XX__CHIP_SELECT,              // UINT8  CPU_MEMORY_CONFIG::ChipSelect;
        TRUE,                              // UINT8  CPU_MEMORY_CONFIG::ReadOnly;
        LPC43XX__WAIT_STATES,              // UINT32 CPU_MEMORY_CONFIG::WaitStates;
        LPC43XX__RELEASE_COUNTS,           // UINT32 CPU_MEMORY_CONFIG::ReleaseCounts;
        LPC43XX__BIT_WIDTH,                // UINT32 CPU_MEMORY_CONFIG::BitWidth;
        LPC43XX__BASE_ADDRESS,             // UINT32 CPU_MEMORY_CONFIG::BaseAddress;
        LPC43XX__SIZE_IN_BYTES,            // UINT32 CPU_MEMORY_CONFIG::SizeInBytes;
        0,                                 // UINT8  CPU_MEMORY_CONFIG::XREADYEnable 
        0,                                 // UINT8  CPU_MEMORY_CONFIG::ByteSignalsForRead 
        0,                                 // UINT8  CPU_MEMORY_CONFIG::ExternalBufferEnable
    },

    0,                                     // UINT32 ChipProtection;
    FLASH_MANUFACTURER_CODE,               // UINT32 ManufacturerCode;
    FLASH_DEVICE_CODE,                     // UINT32 DeviceCode;
};

#if defined(ADS_LINKER_BUG__NOT_ALL_UNUSED_VARIABLES_ARE_REMOVED)
#pragma arm section rodata = "g_LPC43XX_BS"
#endif

struct BlockStorageDevice g_LPC43XX_BS;

#if defined(ADS_LINKER_BUG__NOT_ALL_UNUSED_VARIABLES_ARE_REMOVED)
#pragma arm section rodata 
#endif 
