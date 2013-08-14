////////////////////////////////////////////////////////////////////////////////
// LPC43XX_SPIFI.cpp - SPIFI flash driver for NXP LPC43XX
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
// Ported to NXP LPC43XX by Micromint USA <support@micromint.com>
////////////////////////////////////////////////////////////////////////////////

#include <tinyhal.h>
#include "LPC43XX.h"

#include "LPC43XX_SPIFI.h"

#define SPIFI_MEM_BASE    0x14000000

// Winbond W25Q32FV / W25Q64FV
#define SPIFI_BLOCK_SIZE    (64 * 1024)
#define SPIFI_BLOCK_MASK    (SPIFI_BLOCK_SIZE - 1)
#define SPIFI_SECTOR_SIZE   (4 * 1024)
#define SPIFI_SECTOR_MASK   (SPIFI_SECTOR_SIZE - 1)

#define SPIFI_SECTOR_WORDS  (SPIFI_SECTOR_SIZE / sizeof(UINT32))
#define SPIFI_WRITE_SIZE    64  // 32-bit words (4-byte)

UINT32 sector_buf[SPIFI_SECTOR_WORDS]; // To support partial sector updates

// Local functions
static void SPIFI_SetCmdMode(void);
static void SPIFI_SetMemMode(void);
static void SPIFI_SendCmd(UINT32 Command);
static void SPIFI_WriteSector(UINT32* dstAddr, UINT32* srcAddr, UINT32 nBytes);

#pragma arm section code = "SectionForFlashOperations"
// ---------------------------------------------------------------------------
BOOL LPC43XX_SPIFI_Driver::ChipInitialize(void* context)
{
    NATIVE_PROFILE_HAL_DRIVERS_FLASH();
    return TRUE;
}
#pragma arm section code

// ---------------------------------------------------------------------------
BOOL LPC43XX_SPIFI_Driver::ChipUnInitialize(void* context)
{
    NATIVE_PROFILE_HAL_DRIVERS_FLASH();
    return TRUE;
}

// ---------------------------------------------------------------------------
const BlockDeviceInfo* LPC43XX_SPIFI_Driver::GetDeviceInfo(void* context)
{
    MEMORY_MAPPED_NOR_BLOCK_CONFIG* config = (MEMORY_MAPPED_NOR_BLOCK_CONFIG*)context;
    
    return config->BlockConfig.BlockDeviceInformation;    
}

// ---------------------------------------------------------------------------
BOOL LPC43XX_SPIFI_Driver::Read(void* context, ByteAddress StartSector, UINT32 NumBytes, BYTE * pSectorBuff)
{
    // XIP device does not need to read into a buffer
    NATIVE_PROFILE_HAL_DRIVERS_FLASH();

    if (pSectorBuff == NULL) return FALSE;

    UINT32* ChipAddress = (UINT32 *)StartSector;
    UINT32* EndAddress  = (UINT32 *)(StartSector + NumBytes);
    UINT32 *pBuf        = (UINT32 *)pSectorBuff;

    while(ChipAddress < EndAddress)
    {
        *pBuf++ = *ChipAddress++;
    }

    return TRUE;
}

#pragma arm section code = "SectionForFlashOperations"
// These functions change SPIFI flash and need to be relocated to run
// from SRAM. Any functions invoked here also need to be in SRAM.

// ---------------------------------------------------------------------------
BOOL LPC43XX_SPIFI_Driver::Write(void* context, ByteAddress Address,
                                            UINT32 NumBytes, BYTE * pSectorBuff, BOOL ReadModifyWrite)
{
    NATIVE_PROFILE_PAL_FLASH();

    // Read-modify-write is used for FAT filesystems only
    if (ReadModifyWrite) return FALSE;

    UINT32* ChipAddress = (UINT32 *)(Address - SPIFI_MEM_BASE);
    UINT32* EndAddress  = (UINT32 *)(ChipAddress + NumBytes);
    UINT32* pBuf = (UINT32 *)pSectorBuff;
    UINT32 nBytes;
    
    SPIFI_SetCmdMode();

    while (ChipAddress < EndAddress) {
        nBytes = SPIFI_SECTOR_SIZE - ((UINT32)ChipAddress % SPIFI_SECTOR_SIZE);
        if (nBytes > (UINT32)(EndAddress - ChipAddress)) nBytes = (UINT32)(EndAddress - ChipAddress);
        SPIFI_WriteSector(ChipAddress, pBuf, nBytes);
        ChipAddress += nBytes;
        pBuf += nBytes;
    }

    SPIFI_SetMemMode();
    return TRUE;
}

// ---------------------------------------------------------------------------
BOOL LPC43XX_SPIFI_Driver::EraseBlock(void* context, ByteAddress Address)
{
    NATIVE_PROFILE_HAL_DRIVERS_FLASH();

    UINT8 stat;

    SPIFI_SetCmdMode();

    LPC_SPIFI->ADDR = (Address - SPIFI_MEM_BASE) & ~SPIFI_BLOCK_MASK; // Start of block relative to base
    SPIFI_SendCmd(SPIFI_WRITE_ENABLE);
    SPIFI_SendCmd(SPIFI_WRITE_ENABLE); // Fix
    SPIFI_SendCmd(SPIFI_ERASE_BLOCK);
    SPIFI_SendCmd(SPIFI_READ_STATUS);
    stat = LPC_SPIFI->DATA_BYTE; // status

    SPIFI_SetMemMode();
    return TRUE;
}

// ---------------------------------------------------------------------------
static void SPIFI_SetCmdMode(void)
{
    __disable_irq();
    SCnSCB->ACTLR &= ~2; // Disable Cortex write buffer 
    //LPC_CGU->BASE_CLK[CLK_BASE_SPIFI] = ((1 << 11) | (CLKIN_IRC << 24)); // Change clock to IRC
    if (LPC_SPIFI->STAT & STAT_MCINIT)  // In memory mode?
    {
        LPC_SPIFI->STAT = STAT_RESET; // Go to command mode
        while (LPC_SPIFI->STAT & STAT_RESET); // Wait for completion
    }
    LPC_SPIFI->IDATA = IDATA_OPCODE; // Disable no-opcode mode
}

// ---------------------------------------------------------------------------
static void SPIFI_SetMemMode(void)
{
    LPC_SPIFI->IDATA = IDATA_NO_OPCODE; // Enable no-opcode mode
    SPIFI_SendCmd(SPIFI_READ_QUAD);
    LPC_SPIFI->MCMD = SPIFI_CMD(CMD_READ_QUAD, CMD_FRAME_03, 0x02, (0x03 << 16));
    //LPC_CGU->BASE_CLK[CLK_BASE_SPIFI] = ((1 << 11) | (CLKIN_IDIVE << 24)); // Restore clock
    SCnSCB->ACTLR |= 2; // Enable Cortex write buffer
    __enable_irq();
}

// ---------------------------------------------------------------------------
static void SPIFI_SendCmd(UINT32 Command)
{
    LPC_SPIFI->CMD = Command;
    while (LPC_SPIFI->STAT & STAT_CMD); // Wait for completion
}

// ---------------------------------------------------------------------------
static void SPIFI_WriteSector(UINT32 *dstAddr, UINT32 *srcAddr, UINT32 nBytes)
{
    UINT32 addr = (UINT32)dstAddr & ~(SPIFI_SECTOR_MASK);
    UINT32 offset = (UINT32)dstAddr & SPIFI_SECTOR_MASK;
    UINT32 *pBuf = srcAddr;
    UINT8 stat;
    int i, j;

    LPC_SPIFI->ADDR = addr;

    // Read sector into buffer is only updating part of it
    if (offset != 0 || nBytes < SPIFI_SECTOR_SIZE) {
        LPC_SPIFI->CMD = SPIFI_READ_BLOCK | SPIFI_SECTOR_SIZE;
        for(i = 0; i < SPIFI_SECTOR_WORDS; i++)
            sector_buf[i] = LPC_SPIFI->DATA;
            //sector_buf[i] = *(volatile UINT32*)&LPC_SPIFI->DATA;
        while (LPC_SPIFI->STAT & STAT_CMD); // Wait for completion
        i = (offset + nBytes)/4;
        for (j = offset/4; j < i; j++)
            sector_buf[j] = *srcAddr++;
        pBuf = sector_buf;
    }

    // Erase sector
    SPIFI_SendCmd(SPIFI_WRITE_ENABLE);
    SPIFI_SendCmd(SPIFI_ERASE_SECTOR);
    SPIFI_SendCmd(SPIFI_READ_STATUS);
    stat = LPC_SPIFI->DATA_BYTE; // status

    // Program Sector
    for (i = 0; i < SPIFI_SECTOR_WORDS; i += SPIFI_WRITE_SIZE) {
        SPIFI_SendCmd(SPIFI_WRITE_ENABLE);
        LPC_SPIFI->CMD = SPIFI_PROG | (SPIFI_WRITE_SIZE << 2);
        for (j = 0; j < SPIFI_WRITE_SIZE; j++)
            LPC_SPIFI->DATA = pBuf[i + j];
        while (LPC_SPIFI->STAT & 2); // Wait for completion
        SPIFI_SendCmd(SPIFI_READ_STATUS);
        stat = LPC_SPIFI->DATA_BYTE; // status
        LPC_SPIFI->ADDR += (SPIFI_WRITE_SIZE << 2);
    }
}

#pragma arm section code

// ---------------------------------------------------------------------------
BOOL LPC43XX_SPIFI_Driver::IsBlockErased(void* context, ByteAddress BlockStart, UINT32 BlockLength)
{
    NATIVE_PROFILE_HAL_DRIVERS_FLASH();
    
    UINT32* ChipAddress = (UINT32 *) BlockStart;
    UINT32* EndAddress  = (UINT32 *)(BlockStart + BlockLength);
    
    while(ChipAddress < EndAddress)
    {
        if(*ChipAddress != (UINT32)-1)
        {
            return FALSE;
        }
        ChipAddress++;
    }

    return TRUE;
}

// ---------------------------------------------------------------------------
BOOL LPC43XX_SPIFI_Driver::Memset(void* context, ByteAddress Address, UINT8 Data, UINT32 NumBytes)
{
    NATIVE_PROFILE_PAL_FLASH();

    // ToDo: Implement SPIFI filesystem support
    return FALSE;
}

// ---------------------------------------------------------------------------
BOOL LPC43XX_SPIFI_Driver::GetSectorMetadata(void* context, ByteAddress SectorStart, SectorMetadata* pSectorMetadata)
{
    // ToDo: Implement SPIFI filesystem support
    return FALSE;
}

// ---------------------------------------------------------------------------
BOOL LPC43XX_SPIFI_Driver::SetSectorMetadata(void* context, ByteAddress SectorStart, SectorMetadata* pSectorMetadata)
{
    // ToDo: Implement SPIFI filesystem support
    return FALSE;
}

// ---------------------------------------------------------------------------
void LPC43XX_SPIFI_Driver::SetPowerState(void* context, UINT32 State)
{
}

// ---------------------------------------------------------------------------
UINT32 LPC43XX_SPIFI_Driver::MaxSectorWrite_uSec(void* context)
{
    NATIVE_PROFILE_PAL_FLASH();

    MEMORY_MAPPED_NOR_BLOCK_CONFIG* config = (MEMORY_MAPPED_NOR_BLOCK_CONFIG*)context;
    
    return config->BlockConfig.BlockDeviceInformation->MaxSectorWrite_uSec;
}

// ---------------------------------------------------------------------------
UINT32 LPC43XX_SPIFI_Driver::MaxBlockErase_uSec(void* context)
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
