////////////////////////////////////////////////////////////////////////////////
// LPC43XX_SPIFI.cpp - SPIFI flash driver for Micromint Bambino 200
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
#include "LPC43XX.h"
#include "LPC43XX_SPIFI.h"

typedef UINT16 CHIP_WORD;

#define FLASH_CR_PSIZE_BITS FLASH_CR_PSIZE_0 // 16 bit programming
    
static const int LPC43XX_SPIFI_KEY1 = 0x45670123;
static const int LPC43XX_SPIFI_KEY2 = 0xcdef89ab;

BOOL __section(SectionForFlashOperations)LPC43XX_SPIFI_Driver::ChipInitialize( void* context )
{
    NATIVE_PROFILE_HAL_DRIVERS_FLASH();
    return TRUE;
}

BOOL __section(SectionForFlashOperations)LPC43XX_SPIFI_Driver::ChipUnInitialize( void* context )
{
    NATIVE_PROFILE_HAL_DRIVERS_FLASH();
    return TRUE;
}

const BlockDeviceInfo* __section(SectionForFlashOperations)LPC43XX_SPIFI_Driver::GetDeviceInfo( void* context )
{
    MEMORY_MAPPED_NOR_BLOCK_CONFIG* config = (MEMORY_MAPPED_NOR_BLOCK_CONFIG*)context;
    
    return config->BlockConfig.BlockDeviceInformation;    
}

BOOL  __section(SectionForFlashOperations)LPC43XX_SPIFI_Driver::Read( void* context, ByteAddress StartSector, UINT32 NumBytes, BYTE * pSectorBuff)
{
    // XIP device does not need to read into a buffer
    NATIVE_PROFILE_HAL_DRIVERS_FLASH();

    if (pSectorBuff == NULL) return FALSE;

    CHIP_WORD* ChipAddress = (CHIP_WORD *)StartSector;
    CHIP_WORD* EndAddress  = (CHIP_WORD *)(StartSector + NumBytes);
    CHIP_WORD *pBuf        = (CHIP_WORD *)pSectorBuff;

    while(ChipAddress < EndAddress)
    {
        *pBuf++ = *ChipAddress++;
    }

    return TRUE;
}

BOOL __section(SectionForFlashOperations)LPC43XX_SPIFI_Driver::Write(void* context, ByteAddress Address, UINT32 NumBytes, BYTE * pSectorBuff, BOOL ReadModifyWrite)
{
    NATIVE_PROFILE_PAL_FLASH();

    return FALSE;
}

BOOL __section(SectionForFlashOperations)LPC43XX_SPIFI_Driver::Memset(void* context, ByteAddress Address, UINT8 Data, UINT32 NumBytes)
{
    NATIVE_PROFILE_PAL_FLASH();
    // used for FAT filesystems only
    return FALSE;
}

BOOL __section(SectionForFlashOperations)LPC43XX_SPIFI_Driver::GetSectorMetadata(void* context, ByteAddress SectorStart, SectorMetadata* pSectorMetadata)
{
    // no metadata
    return FALSE;
}

BOOL __section(SectionForFlashOperations)LPC43XX_SPIFI_Driver::SetSectorMetadata(void* context, ByteAddress SectorStart, SectorMetadata* pSectorMetadata)
{
    // no metadata
    return FALSE;
}

BOOL __section(SectionForFlashOperations)LPC43XX_SPIFI_Driver::IsBlockErased( void* context, ByteAddress BlockStart, UINT32 BlockLength )
{
    NATIVE_PROFILE_HAL_DRIVERS_FLASH();
    
    CHIP_WORD* ChipAddress = (CHIP_WORD *) BlockStart;
    CHIP_WORD* EndAddress  = (CHIP_WORD *)(BlockStart + BlockLength);
    
    while(ChipAddress < EndAddress)
    {
        if(*ChipAddress != (CHIP_WORD)-1)
        {
            return FALSE;
        }
        ChipAddress++;
    }

    return TRUE;
}

BOOL __section(SectionForFlashOperations)LPC43XX_SPIFI_Driver::EraseBlock( void* context, ByteAddress Sector )
{
    NATIVE_PROFILE_HAL_DRIVERS_FLASH();

    return FALSE;
}

void  __section(SectionForFlashOperations)LPC43XX_SPIFI_Driver::SetPowerState( void* context, UINT32 State )
{
}

#pragma arm section code = "SectionForFlashOperations"

UINT32 __section(SectionForFlashOperations)LPC43XX_SPIFI_Driver::MaxSectorWrite_uSec( void* context )
{
    NATIVE_PROFILE_PAL_FLASH();

    MEMORY_MAPPED_NOR_BLOCK_CONFIG* config = (MEMORY_MAPPED_NOR_BLOCK_CONFIG*)context;
    
    return config->BlockConfig.BlockDeviceInformation->MaxSectorWrite_uSec;
}

UINT32 __section(SectionForFlashOperations)LPC43XX_SPIFI_Driver::MaxBlockErase_uSec( void* context )
{
    NATIVE_PROFILE_PAL_FLASH();
    
    MEMORY_MAPPED_NOR_BLOCK_CONFIG* config = (MEMORY_MAPPED_NOR_BLOCK_CONFIG*)context;

    return config->BlockConfig.BlockDeviceInformation->MaxBlockErase_uSec;
}

#if defined(ADS_LINKER_BUG__NOT_ALL_UNUSED_VARIABLES_ARE_REMOVED)
#pragma arm section rodata = "g_LPC43XX_SPIFI_DeviceTable"
#endif

struct IBlockStorageDevice g_LPC43XX_SPIFI_DeviceTable = 
{                          
    &LPC43XX_SPIFI_Driver::ChipInitialize,
    &LPC43XX_SPIFI_Driver::ChipUnInitialize,
    &LPC43XX_SPIFI_Driver::GetDeviceInfo,
    &LPC43XX_SPIFI_Driver::Read,
    &LPC43XX_SPIFI_Driver::Write,
    &LPC43XX_SPIFI_Driver::Memset,    
    &LPC43XX_SPIFI_Driver::GetSectorMetadata,
    &LPC43XX_SPIFI_Driver::SetSectorMetadata,
    &LPC43XX_SPIFI_Driver::IsBlockErased,
    &LPC43XX_SPIFI_Driver::EraseBlock,
    &LPC43XX_SPIFI_Driver::SetPowerState,
    &LPC43XX_SPIFI_Driver::MaxSectorWrite_uSec,
    &LPC43XX_SPIFI_Driver::MaxBlockErase_uSec,    
};

#if defined(ADS_LINKER_BUG__NOT_ALL_UNUSED_VARIABLES_ARE_REMOVED)
#pragma arm section rodata 
#endif 
