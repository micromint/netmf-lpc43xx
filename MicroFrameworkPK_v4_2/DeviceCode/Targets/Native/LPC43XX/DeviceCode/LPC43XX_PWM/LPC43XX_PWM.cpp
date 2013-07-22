////////////////////////////////////////////////////////////////////////////////
// LPC43XX_PWM.cpp - PWM functions for NXP LPC43XX
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

BOOL PWM_Initialize(PWM_CHANNEL channel)
{
    return TRUE;
}

BOOL PWM_Uninitialize(PWM_CHANNEL channel)
{
    return TRUE;
}

BOOL PWM_ApplyConfiguration(PWM_CHANNEL channel, GPIO_PIN pin, UINT32& period, UINT32& duration, PWM_SCALE_FACTOR& scale, BOOL invert)
{
    return TRUE;
}

BOOL PWM_Start(PWM_CHANNEL channel, GPIO_PIN pin)
{
    return TRUE;
}

void PWM_Stop(PWM_CHANNEL channel, GPIO_PIN pin)
{
}

BOOL PWM_Start(PWM_CHANNEL* channel, GPIO_PIN* pin, UINT32 count)
{
    return TRUE;
}

void PWM_Stop(PWM_CHANNEL* channel, GPIO_PIN* pin, UINT32 count)
{
}

UINT32 PWM_PWMChannels() 
{
    return 0;
}

GPIO_PIN PWM_GetPinForChannel( PWM_CHANNEL channel )
{
    return GPIO_PIN_NONE;
}
