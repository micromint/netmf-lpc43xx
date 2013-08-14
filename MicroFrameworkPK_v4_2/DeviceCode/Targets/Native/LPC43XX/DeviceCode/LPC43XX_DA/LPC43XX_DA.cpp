////////////////////////////////////////////////////////////////////////////////
// LPC43XX_DA.cpp - DAC functions for NXP LPC43XX
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
#include "LPC43XX_PINS.h"

#define TOTAL_DA_CHAN  1
#define MAX_DA_PREC    10

static GPIO_PIN const __section(rodata) DA_Pin[] = {(GPIO_PIN)P_DAC0};

// ---------------------------------------------------------------------------
BOOL DA_Initialize(DA_CHANNEL channel, INT32 precisionInBits)
{
    if ((UINT32)channel >= TOTAL_DA_CHAN) return FALSE;

    // Configure the pin as GPIO input
    PIN_Config(DA_Pin[channel], (SCU_PINIO_PULLNONE | 0x0));
    // Select DAC on analog function select register in SCU
    LPC_SCU->ENAIO[2] |= 1; // Sets pin P4_4 as DAC

    // Set Maximum update rate for DAC */
    LPC_DAC->CR &= ~DAC_BIAS_EN;
	
    DA_Write(channel, 0);
    return TRUE;
}

// ---------------------------------------------------------------------------
void DA_Write(DA_CHANNEL channel, INT32 level)
{
     uint32_t tmp;

    if ((UINT32)channel >= TOTAL_DA_CHAN) return;

    // Set the DAC output
    tmp = LPC_DAC->CR & DAC_BIAS_EN;
    tmp |= DAC_VALUE(level);
    LPC_DAC->CR = tmp;
}

// ---------------------------------------------------------------------------
UINT32 DA_DAChannels()
{
    return TOTAL_DA_CHAN;
}

// ---------------------------------------------------------------------------
GPIO_PIN DA_GetPinForChannel(DA_CHANNEL channel)
{
    if ((UINT32)channel >= TOTAL_DA_CHAN) return GPIO_PIN_NONE;

    return DA_Pin[channel];
}

// ---------------------------------------------------------------------------
BOOL DA_GetAvailablePrecisionsForChannel(DA_CHANNEL channel, INT32* precisions, UINT32& size)
{
    size = 0;
    if (precisions == NULL || (UINT32)channel >= TOTAL_DA_CHAN) return FALSE;

    precisions[0] = MAX_DA_PREC;
    size = 1;
    return TRUE;
}
