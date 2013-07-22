////////////////////////////////////////////////////////////////////////////////
// LPC43XX_SPIFI.h - SPIFI declarations for Micromint Bambino 200
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

//--//
#ifndef _LPC43XX_SPIFI_H_
#define _LPC43XX_SPIFI_H_

//--//

struct LPC43XX_SPIFI_Driver
{
    static BOOL ChipInitialize( void* context );
    static BOOL ChipUnInitialize( void* context );
    static const BlockDeviceInfo* GetDeviceInfo( void* context );
    static BOOL Read( void* context, ByteAddress Address, UINT32 NumBytes, BYTE * pSectorBuff);
    static BOOL Write( void* context, ByteAddress Address, UINT32 NumBytes, BYTE * pSectorBuff, BOOL ReadModifyWrite);
    static BOOL Memset( void* context, ByteAddress Address, UINT8 Data, UINT32 NumBytes );
    static BOOL GetSectorMetadata(void* context, ByteAddress SectorStart, SectorMetadata* pSectorMetadata);
    static BOOL SetSectorMetadata(void* context, ByteAddress SectorStart, SectorMetadata* pSectorMetadata);
    static BOOL IsBlockErased( void* context, ByteAddress Address, UINT32 BlockLength );
    static BOOL EraseBlock( void* context, ByteAddress Address );
    static void SetPowerState( void* context, UINT32 State );
    static UINT32 MaxSectorWrite_uSec(void* context);
    static UINT32 MaxBlockErase_uSec(void* context);
    static BOOL ChipReadOnly( void* context, BOOL On, UINT32 ProtectionKey );
    static BOOL ReadProductID( void* context, FLASH_WORD& ManufacturerCode, FLASH_WORD& DeviceCode );
};

#endif // _LPC43XX_SPIFI_H_
