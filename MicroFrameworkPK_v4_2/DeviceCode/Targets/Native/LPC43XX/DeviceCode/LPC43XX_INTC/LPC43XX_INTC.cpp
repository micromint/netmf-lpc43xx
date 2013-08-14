////////////////////////////////////////////////////////////////////////////////
// LPC43XX_INTC.cpp - INTC functions for NXP LPC43XX
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

#define NVIC_CORE_VECTORS         (16)    // Minimal ARM core vectors

extern UINT32 ARM_Vectors;
static volatile UINT32* _vectors = &ARM_Vectors;

// ---------------------------------------------------------------------------
static BOOL NVIC_IRQEnableState(IRQn_Type IRQn)
{
  return (BOOL)((NVIC->ISER[(UINT32)((INT32)IRQn) >> 5] >>
                    ((UINT32)((INT32)IRQn) & (UINT32)0x1F)) & 1);
}

// ---------------------------------------------------------------------------
void CPU_INTC_Initialize()
{
    int i;

    // Clear interrupts
    NVIC->ICER[0] = 0xFFFFFFFF;
    NVIC->ICER[1] = 0xFFFFFFFF;
    NVIC->ICER[2] = 0xFFFFFFFF;

    // Copy and switch to dynamic vectors
    if (SCB->VTOR != (UINT32)_vectors)
    {
        UINT32 *old_vectors = (UINT32*)SCB->VTOR;
        for (i = 0; i < NVIC_CORE_VECTORS; i++) { // Minimal table at FirstEntry.s
            _vectors[i] = old_vectors[i];
        }
        SCB->VTOR = (UINT32)_vectors;
    }
}

// ---------------------------------------------------------------------------
// Note: Cortex-M vectors don't pass parameters so ISR_Param is ignored
// ToDo: Consider using SV handler?
BOOL CPU_INTC_ActivateInterrupt(UINT32 Irq_Index, HAL_CALLBACK_FPN ISR, void* ISR_Param)
{
    _vectors[Irq_Index + 16] = (UINT32)ISR;
    NVIC_ClearPendingIRQ((IRQn_Type) Irq_Index);
    NVIC_EnableIRQ((IRQn_Type) Irq_Index);
    return TRUE;
}

// ---------------------------------------------------------------------------
BOOL CPU_INTC_DeactivateInterrupt(UINT32 Irq_Index)
{
    NVIC_DisableIRQ((IRQn_Type) Irq_Index);
    return TRUE;
}

// ---------------------------------------------------------------------------
BOOL CPU_INTC_InterruptEnable(UINT32 Irq_Index)
{
    BOOL wasEnabled = NVIC_IRQEnableState((IRQn_Type) Irq_Index);
    NVIC_EnableIRQ((IRQn_Type) Irq_Index);
    return wasEnabled;
}

// ---------------------------------------------------------------------------
BOOL CPU_INTC_InterruptDisable(UINT32 Irq_Index)
{
    BOOL wasEnabled = NVIC_IRQEnableState((IRQn_Type) Irq_Index);
    NVIC_DisableIRQ((IRQn_Type) Irq_Index);
    return wasEnabled;
}

// ---------------------------------------------------------------------------
BOOL CPU_INTC_InterruptEnableState(UINT32 Irq_Index)
{
    return NVIC_IRQEnableState((IRQn_Type) Irq_Index);
}

// ---------------------------------------------------------------------------
BOOL CPU_INTC_InterruptState(UINT32 Irq_Index)
{
    // Return TRUE if interrupt status is pending.
    return (BOOL)NVIC_GetPendingIRQ((IRQn_Type) Irq_Index);
}
