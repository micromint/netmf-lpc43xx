////////////////////////////////////////////////////////////////////////////////
// LPC43XX_AD.cpp - ADC functions for NXP LPC43XX
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

#define TOTAL_AD_CHAN  6
#define MAX_AD_PREC    10

static inline int div_round_up(int x, int y)
{
  return (x + (y - 1)) / y;
}

// Only supports ADC0 channels. Will eventually support ADC1 channels.
static GPIO_PIN const AD_Pin[] = {(GPIO_PIN)P_ADC0, (GPIO_PIN)P_ADC1, (GPIO_PIN)P_ADC2,
                                  (GPIO_PIN)P_ADC3, (GPIO_PIN)P_ADC4, (GPIO_PIN)P_ADC5};

BOOL AD_Initialize(ANALOG_CHANNEL channel, INT32 precisionInBits)
{
    GPIO_PIN pin = AD_GetPinForChannel(channel);
    uint8_t num, chan;

    if ((UINT32)channel >= TOTAL_AD_CHAN) return FALSE;

    // Configure the pin as GPIO input
    if (pin < SFP_AIO0) {
        PIN_Config(pin, (SCU_PINIO_PULLNONE | 0x0));
        num = 0;
        chan = (uint8_t)(pin % 7);
    } else {
        num = LPC43XX_ADC_NUM(pin);
        chan = LPC43XX_ADC_CHAN(pin);
    }

    // Calculate minimum clock divider
    //  clkdiv = divider - 1
		uint32_t PCLK = SystemCoreClock;
    uint32_t adcRate = 400000;
    uint32_t clkdiv = div_round_up(PCLK, adcRate) - 1;
    
    // Set the generic software-controlled ADC settings
    LPC_ADC0->CR = (0 << 0)      // SEL: 0 = no channels selected
                  | (clkdiv << 8) // CLKDIV:
                  | (0 << 16)     // BURST: 0 = software control
                  | (1 << 21)     // PDN: 1 = operational
                  | (0 << 24)     // START: 0 = no start
                  | (0 << 27);    // EDGE: not applicable

    // Select ADC on analog function select register in SCU
    LPC_SCU->ENAIO[num] |= 1UL << chan;

    return TRUE;
}

INT32 AD_Read(ANALOG_CHANNEL channel)
{
    // Select the appropriate channel and start conversion
    LPC_ADC0->CR &= ~0xFF;
    LPC_ADC0->CR |= 1 << (int)channel;
    LPC_ADC0->CR |= 1 << 24;

    // Repeatedly get the sample data until DONE bit
    unsigned int data;
    do {
        data = LPC_ADC0->GDR;
    } while ((data & ((unsigned int)1 << 31)) == 0);

    // Stop conversion
    LPC_ADC0->CR &= ~(1 << 24);

    return (data >> 6) & ADC_RANGE; // 10 bit
}

UINT32 AD_ADChannels()
{
    return TOTAL_AD_CHAN;
}

GPIO_PIN AD_GetPinForChannel(ANALOG_CHANNEL channel)
{
    if ((UINT32)channel >= TOTAL_AD_CHAN) return GPIO_PIN_NONE;
 
    return AD_Pin[channel];
}

BOOL AD_GetAvailablePrecisionsForChannel( ANALOG_CHANNEL channel, INT32* precisions, UINT32& size )
{
    size = 0;
    if (precisions == NULL || (UINT32)channel >= TOTAL_AD_CHAN) return FALSE;

    precisions[0] = MAX_AD_PREC;
    size = 1;
    return TRUE;
}
