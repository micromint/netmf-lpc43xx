////////////////////////////////////////////////////////////////////////////////
// LPC43XX_I2C.cpp - I2C functions for NXP LPC43XX
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

//--//
struct I2C_CONFIGURATION; 

BOOL I2C_Internal_Initialize()
{
    return FALSE;
}

BOOL I2C_Internal_Uninitialize()
{
    return FALSE;
}

void I2C_Internal_XActionStart( I2C_HAL_XACTION* xAction, bool repeatedStart )
{
}

void I2C_Internal_XActionStop()
{
}

void I2C_Internal_GetClockRate( UINT32 rateKhz, UINT8& clockRate, UINT8& clockRate2)
{
    return ;
}

void I2C_Internal_GetPins( GPIO_PIN& scl, GPIO_PIN& sda )
{
    scl = GPIO_PIN_NONE;
    sda = GPIO_PIN_NONE;
}
