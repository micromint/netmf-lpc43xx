////////////////////////////////////////////////////////////////////////////////
// LPC43XX_Time.cpp - Time functions for NXP LPC43XX
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
// See the License for the specific language governing permissions andSYSTIMER-
// limitations under the License.
//
// Copyright (c) Microsoft Corporation.  All rights reserved.
// Ported to NXP LPC43XX by Micromint USA <support@micromint.com>
////////////////////////////////////////////////////////////////////////////////

#include <tinyhal.h>
#include "LPC43XX.h"

// Select system timer (default=TIMER3)
#define SYSTIMER          LPC_TIMER3
#define SYSTIMER_IRQn     TIMER3_IRQn

static UINT64 g_lastCount;
static UINT64 g_nextEvent;

void systimer_handler(void* param);

// ---------------------------------------------------------------------------
BOOL HAL_Time_Initialize()
{
    g_lastCount = 0;
    g_nextEvent = 0x0000FFFFFFFFFFFF;

    SYSTIMER->CTCR = 0x0; // Set timer mode
    SYSTIMER->TCR = 0x2; // Reset timer

    // Set prescaler for a microsecond timer (1 MHz -> 1 us ticks)
    SYSTIMER->PR = (SYSTEM_CLOCK_HZ / ONE_MHZ) - 1;
    SYSTIMER->TCR = 0x1; // Enable timer

    CPU_INTC_ActivateInterrupt(SYSTIMER_IRQn, systimer_handler, 0);

    return TRUE;
}

// ---------------------------------------------------------------------------
BOOL HAL_Time_Uninitialize()
{
    SYSTIMER->MCR &= ~1; // Disable interrupt
    CPU_INTC_DeactivateInterrupt(SYSTIMER_IRQn);
    SYSTIMER->TCR = 0x0; // Disable timer
    return TRUE;
}

// ---------------------------------------------------------------------------
UINT64 HAL_Time_CurrentTicks()
{
    UINT32 count = SYSTIMER->TC;
    UINT32 highCount = (UINT32)(g_lastCount >> 32);
    UINT32 lowCount = (UINT32)(g_lastCount & 0xFFFFFFFF);

    // Increment high count on timer overflow
    if (count < lowCount)
    {
        highCount++;
    }

    // Update last count and return
    g_lastCount = (((UINT64)highCount << 32) | count);

    return g_lastCount;
}

// ---------------------------------------------------------------------------
INT64 HAL_Time_TicksToTime(UINT64 Ticks)
{
    return CPU_TicksToTime(Ticks);
}

// ---------------------------------------------------------------------------
INT64 HAL_Time_CurrentTime()
{
    return CPU_TicksToTime(HAL_Time_CurrentTicks());
}

// ---------------------------------------------------------------------------
void HAL_Time_SetCompare(UINT64 CompareValue)
{
    GLOBAL_LOCK(irq);
    g_nextEvent = CompareValue;
    SYSTIMER->MR[0] = (UINT32)CompareValue;
    SYSTIMER->MCR |= 1; // Enable match interrupt

    if (HAL_Time_CurrentTicks() >= CompareValue) { // Missed event?
        // If so, trigger immediate interrupt
        NVIC->STIR = SYSTIMER_IRQn;
    }
}

//
// To calibrate this constant, uncomment #define CALIBRATE_SLEEP_USEC in TinyHAL.c
//
#define CPU_SLEEP_USEC_FIXED_OVERHEAD_CLOCKS 3

// ---------------------------------------------------------------------------
void HAL_Time_Sleep_MicroSeconds(UINT32 uSec)
{
    GLOBAL_LOCK(irq);

    UINT32 current   = HAL_Time_CurrentTicks();
    UINT32 maxDiff = CPU_MicrosecondsToTicks(uSec);

    if(maxDiff <= CPU_SLEEP_USEC_FIXED_OVERHEAD_CLOCKS) maxDiff = 0; 
    else maxDiff -= CPU_SLEEP_USEC_FIXED_OVERHEAD_CLOCKS;  // Subtract overhead

    while(((INT32)(HAL_Time_CurrentTicks() - current)) <= maxDiff);
}

// This routine is not designed for very accurate time delays. It is designed
// to insure a minimum delay.  It is implemented using a simple timing loop
// that assumes the best of conditions - that is to say: turbo mode, cache,
// and branch prediction enabled, no cache misses and no interrupts while it is
// executing.  Since this cannot be guaranteed, it is possible and even likely
// that this routine will take a bit longer to return than the requested time
// - but it will not return in less time.

// ---------------------------------------------------------------------------
void HAL_Time_Sleep_MicroSeconds_InterruptEnabled(UINT32 uSec)
{
    // iterations must be signed so that negative iterations will result in the minimum delay

    uSec *= ((SYSTEM_CYCLE_CLOCK_HZ / 2) / CLOCK_COMMON_FACTOR);
    uSec /= (ONE_MHZ               / CLOCK_COMMON_FACTOR);

    // iterations is equal to the number of CPU instruction cycles in the required time minus
    // overhead cycles required to call this subroutine.
    int iterations = (int)uSec - 3;      // Subtract off call & calculation overhead

    CYCLE_DELAY_LOOP(iterations);
}

// ---------------------------------------------------------------------------
void systimer_handler(void* param)
{
    GLOBAL_LOCK(irq);

    SYSTIMER->IR = 1; // Clear interrupt

    if (HAL_Time_CurrentTicks() >= g_nextEvent) { // Past event time?
        // Handle it and schedule the next one, if there is one
        HAL_COMPLETION::DequeueAndExec();
    }

}

// ---------------------------------------------------------------------------
void HAL_Time_GetDriftParameters  (INT32* a, INT32* b, INT64* c)
{
    *a = 1;
    *b = 1;
    *c = 0;
}

// ---------------------------------------------------------------------------
UINT32 CPU_SystemClock()
{
    return SYSTEM_CLOCK_HZ;
}

// ---------------------------------------------------------------------------
UINT32 CPU_TicksPerSecond()
{
    return SLOW_CLOCKS_PER_SECOND;
}

// ---------------------------------------------------------------------------
UINT64 CPU_MillisecondsToTicks(UINT64 Ticks)
{
    Ticks *= (SLOW_CLOCKS_PER_SECOND/SLOW_CLOCKS_MILLISECOND_GCD);
    Ticks /= (1000                  /SLOW_CLOCKS_MILLISECOND_GCD);

    return Ticks;
}

// ---------------------------------------------------------------------------
UINT64 CPU_MillisecondsToTicks(UINT32 Ticks32)
{
    UINT64 Ticks;

    Ticks  = (UINT64)Ticks32 * (SLOW_CLOCKS_PER_SECOND/SLOW_CLOCKS_MILLISECOND_GCD);
    Ticks /=                   (1000                  /SLOW_CLOCKS_MILLISECOND_GCD);

    return Ticks;
}

// ---------------------------------------------------------------------------
UINT64 CPU_MicrosecondsToTicks(UINT64 uSec)
{
#if ONE_MHZ <= SLOW_CLOCKS_PER_SECOND
    return uSec * (SLOW_CLOCKS_PER_SECOND / ONE_MHZ);
#else
    return uSec / (ONE_MHZ / SLOW_CLOCKS_PER_SECOND);
#endif
}

// ---------------------------------------------------------------------------
UINT32 CPU_MicrosecondsToTicks(UINT32 uSec)
{
#if ONE_MHZ <= SLOW_CLOCKS_PER_SECOND
    return uSec * (SLOW_CLOCKS_PER_SECOND / ONE_MHZ);
#else
    return uSec / (ONE_MHZ / SLOW_CLOCKS_PER_SECOND);
#endif
}

// ---------------------------------------------------------------------------
UINT32 CPU_MicrosecondsToSystemClocks(UINT32 uSec)
{
    uSec *= (SYSTEM_CLOCK_HZ/CLOCK_COMMON_FACTOR);
    uSec /= (ONE_MHZ        /CLOCK_COMMON_FACTOR);

    return uSec;
}

// ---------------------------------------------------------------------------
int CPU_MicrosecondsToSystemClocks(int uSec)
{
    uSec *= (SYSTEM_CLOCK_HZ/CLOCK_COMMON_FACTOR);
    uSec /= (ONE_MHZ        /CLOCK_COMMON_FACTOR);

    return uSec;
}

// ---------------------------------------------------------------------------
UINT64 CPU_TicksToTime(UINT64 Ticks)
{
    Ticks *= (TEN_MHZ               /SLOW_CLOCKS_TEN_MHZ_GCD);
    Ticks /= (SLOW_CLOCKS_PER_SECOND/SLOW_CLOCKS_TEN_MHZ_GCD);

    return Ticks;
}

// ---------------------------------------------------------------------------
UINT64 CPU_TicksToTime(UINT32 Ticks32)
{
    UINT64 Ticks;

    Ticks  = (UINT64)Ticks32 * (TEN_MHZ               /SLOW_CLOCKS_TEN_MHZ_GCD);
    Ticks /=                   (SLOW_CLOCKS_PER_SECOND/SLOW_CLOCKS_TEN_MHZ_GCD);

    return Ticks;
}
