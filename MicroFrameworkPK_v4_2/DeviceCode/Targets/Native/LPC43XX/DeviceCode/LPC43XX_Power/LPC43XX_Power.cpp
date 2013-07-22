////////////////////////////////////////////////////////////////////////////////
// LPC43XX_Power.cpp - Power functions for NXP LPC43XX
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

void HAL_AssertEx()
{
    // cause an abort and let the abort handler take over
    *((char*)0xFFFFFFFF) = 'a';
}

//--//

BOOL CPU_Initialize()
{
    NATIVE_PROFILE_HAL_PROCESSOR_POWER();
    CPU_INTC_Initialize();
    return TRUE;
}

void CPU_Reset()
{
    NATIVE_PROFILE_HAL_PROCESSOR_POWER();
    SCB->AIRCR = (0x5FA << SCB_AIRCR_VECTKEY_Pos)  // unlock key
               | (1 << SCB_AIRCR_SYSRESETREQ_Pos); // reset request
    while(1); // wait for reset
}

void CPU_Sleep(SLEEP_LEVEL level, UINT64 wakeEvents)
{
    NATIVE_PROFILE_HAL_PROCESSOR_POWER();
    // Enter sleep mode with WFI instruction
    __WFI();
}

void CPU_ChangePowerLevel(POWER_LEVEL level)
{
    switch(level)
    {
        case POWER_LEVEL__MID_POWER:
            break;

        case POWER_LEVEL__LOW_POWER:
            break;

        case POWER_LEVEL__HIGH_POWER:
        default:
            break;
    }
}

BOOL CPU_IsSoftRebootSupported ()
{
    NATIVE_PROFILE_HAL_PROCESSOR_POWER();
    return TRUE;
}


void CPU_Halt()
{
    NATIVE_PROFILE_HAL_PROCESSOR_POWER();
    while(1);
}
