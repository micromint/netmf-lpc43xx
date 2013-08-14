////////////////////////////////////////////////////////////////////////////////
// LPC43XX_SPIFI.h - SPIFI declarations for NXP LPC43XX
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

#ifndef _LPC43XX_SPIFI_H_
#define _LPC43XX_SPIFI_H_

#ifndef USE_SPIFI_LIB

// Command opcodes
#define CMD_ERASE_SECT    0x20
#define CMD_ERASE_BLOCK   0xD8
#define CMD_ERASE_CHIP    0xC7
#define CMD_PROG          0x02
#define CMD_PROG_QUAD     0x32
#define CMD_PROT_SECT     0x36
#define CMD_READ          0x03
#define CMD_READ_ID       0x9F
#define CMD_READ_STAT     0x05
#define CMD_READ_QUAD     0xEB
#define CMD_RESET         0xFF  // Exit QPI
#define CMD_UNPROT_SECT   0x39
#define CMD_WRITE_DIS     0x04
#define CMD_WRITE_EN      0x06
#define CMD_WRITE_STAT    0x01

// Command framing
#define CMD_FRAME_10      0x01  // One opcode, no address bytes
#define CMD_FRAME_11      0x02  // One opcode, one address byte
#define CMD_FRAME_12      0x03  // One opcode, two address bytes
#define CMD_FRAME_13      0x04  // One opcode, three address bytes
#define CMD_FRAME_14      0x05  // One opcode, four address bytes
#define CMD_FRAME_03      0x06  // No opcode,  three address bytes
#define CMD_FRAME_04      0x07  // No opcode,  four address bytes

// Command options
#define CMD_POLL          (1 << 14)
#define CMD_OUTPUT        (1 << 15)

// Encoding for SPIFI command register
#define SPIFI_CMD(op, frm, fld, opt)  (((UINT32)op << 24) | (frm << 21) | (fld << 19) | opt)

// Common SPIFI commands
#define SPIFI_ERASE_BLOCK   SPIFI_CMD(CMD_ERASE_BLOCK, CMD_FRAME_13, 0, 0)
#define SPIFI_ERASE_CHIP    SPIFI_CMD(CMD_ERASE_CHIP,  CMD_FRAME_13, 0, 0)
#define SPIFI_ERASE_SECTOR  SPIFI_CMD(CMD_ERASE_SECT,  CMD_FRAME_13, 0, 0)
#define SPIFI_PROG          SPIFI_CMD(CMD_PROG_QUAD,   CMD_FRAME_13, 1, CMD_OUTPUT) // OR with write size
#define SPIFI_READ_BLOCK    SPIFI_CMD(CMD_READ_QUAD,   CMD_FRAME_03, 2, (3 << 16))  // OR with block size
#define SPIFI_READ_ID       SPIFI_CMD(CMD_READ_ID,     CMD_FRAME_10, 0, 0)
#define SPIFI_READ_QUAD     SPIFI_CMD(CMD_READ_QUAD,   CMD_FRAME_13, 2, (3 << 16))
#define SPIFI_READ_STATUS   SPIFI_CMD(CMD_READ_STAT,   CMD_FRAME_10, 0, CMD_POLL)
#define SPIFI_RESET         SPIFI_CMD(CMD_RESET,       CMD_FRAME_14, 0, (4 << 16))
#define SPIFI_WRITE_ENABLE  SPIFI_CMD(CMD_WRITE_EN,    CMD_FRAME_10, 0, 0)
            
// Status flags
#define STAT_MCINIT       (1 << 0)
#define STAT_CMD          (1 << 1)
#define STAT_RESET        (1 << 4)

// Intermediate data options
#define IDATA_NO_OPCODE   0xA5
#define IDATA_OPCODE      0xFF

#endif // USE_SPIFI_LIB

struct LPC43XX_SPIFI_Driver
{
    static BOOL ChipInitialize(void* context);
    static BOOL ChipUnInitialize(void* context);
    static const BlockDeviceInfo* GetDeviceInfo(void* context);
    static BOOL Read(void* context, ByteAddress Address, UINT32 NumBytes, BYTE * pSectorBuff);
    static BOOL Write(void* context, ByteAddress Address, UINT32 NumBytes, BYTE * pSectorBuff, BOOL ReadModifyWrite);
    static BOOL Memset(void* context, ByteAddress Address, UINT8 Data, UINT32 NumBytes);
    static BOOL GetSectorMetadata(void* context, ByteAddress SectorStart, SectorMetadata* pSectorMetadata);
    static BOOL SetSectorMetadata(void* context, ByteAddress SectorStart, SectorMetadata* pSectorMetadata);
    static BOOL IsBlockErased(void* context, ByteAddress Address, UINT32 BlockLength);
    static BOOL EraseBlock(void* context, ByteAddress Address);
    static void SetPowerState(void* context, UINT32 State);
    static UINT32 MaxSectorWrite_uSec(void* context);
    static UINT32 MaxBlockErase_uSec(void* context);
    static BOOL ChipReadOnly(void* context, BOOL On, UINT32 ProtectionKey);
    static BOOL ReadProductID(void* context, FLASH_WORD& ManufacturerCode, FLASH_WORD& DeviceCode);
};

#endif // _LPC43XX_SPIFI_H_
