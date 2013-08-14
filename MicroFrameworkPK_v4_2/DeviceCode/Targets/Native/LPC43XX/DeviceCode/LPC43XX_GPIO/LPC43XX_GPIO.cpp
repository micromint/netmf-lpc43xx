////////////////////////////////////////////////////////////////////////////////
// LPC43XX_GPIO.cpp - GPIO functions for NXP LPC43XX
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

// On the LPC43xx the MCU pin name and the GPIO pin name are not the same.
// To simplify logic, the Pin identifier is an encoded value with the SCU and
// GPIO data. It is not a sequential number. See LPC43XX_PINS.h for details.
#include "LPC43XX_PINS.h"

#define TOTAL_GPIO_PORT  8
#define TOTAL_GPIO_PINS  (32 * TOTAL_GPIO_PORT)
#define TOTAL_GPIO_INT   8

// GPIO interrupt handlers
void GPIO_IRQHandler(int IrqNum);
void GPIO0_IRQHandler(void* param);
void GPIO1_IRQHandler(void* param);
void GPIO2_IRQHandler(void* param);
void GPIO3_IRQHandler(void* param);
void GPIO4_IRQHandler(void* param);
void GPIO5_IRQHandler(void* param);
void GPIO6_IRQHandler(void* param);
void GPIO7_IRQHandler(void* param);

static UINT32 g_pinReserved[TOTAL_GPIO_PORT]; // 1 bit per pin

typedef struct
{
  UINT8 idx;
  UINT8 ch;
  GPIO_PIN pin;
  GPIO_INTERRUPT_SERVICE_ROUTINE isr;
  void* param;
} GPIO_IRQ_T;

static GPIO_IRQ_T gpio_irq[TOTAL_GPIO_INT];
static const HAL_CALLBACK_FPN gpio_isr[TOTAL_GPIO_INT] = {
    GPIO0_IRQHandler, GPIO1_IRQHandler, GPIO2_IRQHandler, GPIO3_IRQHandler,
    GPIO4_IRQHandler, GPIO5_IRQHandler, GPIO6_IRQHandler, GPIO7_IRQHandler};
static UINT8 g_channel; // Next available GPIO IRQ

// Local functions
static UINT8 GPIO_InitIRQ(GPIO_PIN Pin, GPIO_INTERRUPT_SERVICE_ROUTINE ISR, void* Param);
static void GPIO_SetIRQ(UINT8 ch, GPIO_INT_EDGE IntEdge, BOOL Enable);

// Debounce support
static UINT32 g_debounceTicks;
static HAL_COMPLETION g_completions[TOTAL_GPIO_INT];
static void GPIO_DebounceHandler (void* arg);

// ---------------------------------------------------------------------------
static UINT8 GPIO_InitIRQ(GPIO_PIN Pin, GPIO_INTERRUPT_SERVICE_ROUTINE ISR, void* Param)
{
    UINT8 ch = g_channel; // ToDo: Search for empty one so ch != g_channel
    UINT8 port = LPC43XX_GPIO_PORT(Pin), bit = LPC43XX_GPIO_PIN(Pin);
    GPIO_IRQ_T *obj;

    // Set IRQ data
    obj = &gpio_irq[ch];
    obj->ch = g_channel;
    obj->pin = Pin;
    obj->isr = ISR;
    obj->param = Param;

	  // Clear rising and falling edge detection
    //pmask = (1 << g_channel);
    //LPC_GPIO_PIN_INT->IST = pmask;

    // Set SCU
    if (g_channel < 4) {
        LPC_SCU->PINTSEL0 &= ~(0xFF << (port << 3));
        LPC_SCU->PINTSEL0 |= (((port << 5) | bit) << (g_channel << 3));
    } else {
        LPC_SCU->PINTSEL1 &= ~(0xFF << ((port - 4) << 3));
        LPC_SCU->PINTSEL1 |= (((port << 5) | bit) << ((g_channel - 4) << 3));
    }
	
    CPU_INTC_ActivateInterrupt((IRQn_Type)(PIN_INT0_IRQn + g_channel), gpio_isr[g_channel], (void*) obj);

    // Increment channel number
    g_channel++;
    g_channel %= TOTAL_GPIO_INT;

    return obj->ch;
}

// ---------------------------------------------------------------------------
static void GPIO_SetIRQ(UINT8 ch, GPIO_INT_EDGE IntEdge, BOOL Enable)
{
    uint32_t pmask;

    // Clear pending interrupts
    pmask = (1 << ch);
    LPC_GPIO_PIN_INT->IST = pmask;

    // Configure pin interrupt
    // Rising edge or high level interrupt?
    if (IntEdge == GPIO_INT_EDGE_HIGH
         || IntEdge == GPIO_INT_LEVEL_HIGH
         || IntEdge == GPIO_INT_EDGE_BOTH) {
        if (Enable) {
            LPC_GPIO_PIN_INT->SIENR |= pmask;
        } else {
            LPC_GPIO_PIN_INT->CIENR |= pmask;
        }
    } 

    // Falling edge or low level interrupt?
    if (IntEdge == GPIO_INT_EDGE_LOW
         || IntEdge == GPIO_INT_LEVEL_LOW
         || IntEdge == GPIO_INT_EDGE_BOTH) {
        if (Enable) {
            LPC_GPIO_PIN_INT->SIENF |= pmask;
        } else {
            LPC_GPIO_PIN_INT->CIENF |= pmask;
        }
    }

    // Level or edge?
    if (IntEdge == GPIO_INT_LEVEL_HIGH
         || IntEdge == GPIO_INT_LEVEL_LOW) {
        LPC_GPIO_PIN_INT->ISEL |= pmask;
    } else {
        LPC_GPIO_PIN_INT->ISEL &= ~pmask;
    }
}

// ---------------------------------------------------------------------------
static void GPIO_DebounceHandler (void* arg)
{
}

// ---------------------------------------------------------------------------
void GPIO_IRQHandler(int IrqNum)
{
    GPIO_IRQ_T *obj = &gpio_irq[IrqNum];
    UINT32 mask = (1 << IrqNum);

    obj->isr(obj->pin, CPU_GPIO_GetPinState(obj->pin), obj->param);
    LPC_GPIO_PIN_INT->RISE = mask;
    LPC_GPIO_PIN_INT->FALL = mask;
}

// ---------------------------------------------------------------------------
// Need multiple ISRs since param is ignored
void GPIO0_IRQHandler(void* param) { GPIO_IRQHandler(0); }
void GPIO1_IRQHandler(void* param) { GPIO_IRQHandler(1); }
void GPIO2_IRQHandler(void* param) { GPIO_IRQHandler(2); }
void GPIO3_IRQHandler(void* param) { GPIO_IRQHandler(3); }
void GPIO4_IRQHandler(void* param) { GPIO_IRQHandler(4); }
void GPIO5_IRQHandler(void* param) { GPIO_IRQHandler(5); }
void GPIO6_IRQHandler(void* param) { GPIO_IRQHandler(6); }
void GPIO7_IRQHandler(void* param) { GPIO_IRQHandler(7); }

// ---------------------------------------------------------------------------
BOOL CPU_GPIO_Initialize()
{
    for (int i = 0; i < TOTAL_GPIO_PORT; i++)
    {
        g_pinReserved[i] = 0;
    }
    
    for (int i = 0; i < TOTAL_GPIO_INT; i++) {
        //g_completions[i].InitializeForISR(&GPIO_DebounceHandler);
    }

    return TRUE;
}

// ---------------------------------------------------------------------------
BOOL CPU_GPIO_Uninitialize()
{
    for (int i = 0; i < TOTAL_GPIO_INT; i++)
    {
        g_completions[i].Abort();
    }

    for (int i = 0; i < TOTAL_GPIO_PORT; i++)
    {
        CPU_INTC_DeactivateInterrupt(PIN_INT0_IRQn + i);
    }
    
    return TRUE;
}

// ---------------------------------------------------------------------------
UINT32 CPU_GPIO_Attributes(GPIO_PIN Pin)
{
    return (GPIO_ATTRIBUTE_INPUT | GPIO_ATTRIBUTE_OUTPUT);
}

// ---------------------------------------------------------------------------
void CPU_GPIO_DisablePin(GPIO_PIN Pin, GPIO_RESISTOR ResistorState, UINT32 Direction,
                         GPIO_ALT_MODE AltFunction)
{
    GPIO_IRQ_T *obj;
    PIN_Config(Pin, 0); // Reset to default

    for (int i = 0; i < TOTAL_GPIO_INT; i++)
    {
        *obj = gpio_irq[i];

        if (obj->pin == Pin)
        {
            CPU_INTC_DeactivateInterrupt(PIN_INT0_IRQn + obj->ch);
            obj->pin = GPIO_PIN_NONE;
            obj->isr = NULL;
            obj->param = NULL;
        }
    }
}

// ---------------------------------------------------------------------------
void CPU_GPIO_EnableOutputPin(GPIO_PIN Pin, BOOL InitialState)
{
    LPC_GPIO_T *port_reg = (LPC_GPIO_T *) (LPC_GPIO_PORT_BASE);
    UINT8 port = LPC43XX_GPIO_PORT(Pin), bit = LPC43XX_GPIO_PIN(Pin);
    int f = 0;

    // Configure pin properties
    f = SCU_PINIO_FAST | ((port > 4) ? (4) : (0));
    PIN_Config(Pin, f);
    PIN_Mode(Pin, PullNone);
    port_reg->DIR[port] |= (1 << bit); // Output

    CPU_GPIO_SetPinState(Pin, InitialState);
}

// ---------------------------------------------------------------------------
BOOL CPU_GPIO_EnableInputPin(GPIO_PIN Pin, BOOL GlitchFilterEnable, GPIO_INTERRUPT_SERVICE_ROUTINE ISR,
                             GPIO_INT_EDGE IntEdge, GPIO_RESISTOR ResistorState)
{
    return CPU_GPIO_EnableInputPin2(Pin, GlitchFilterEnable, ISR, 0, IntEdge, ResistorState);
}

// ---------------------------------------------------------------------------
BOOL CPU_GPIO_EnableInputPin2(GPIO_PIN Pin, BOOL GlitchFilterEnable, GPIO_INTERRUPT_SERVICE_ROUTINE ISR,
                              void* ISR_Param, GPIO_INT_EDGE IntEdge, GPIO_RESISTOR ResistorState)
{
    LPC_GPIO_T *port_reg = (LPC_GPIO_T *) (LPC_GPIO_PORT_BASE);
    UINT8 port = LPC43XX_GPIO_PORT(Pin), bit = LPC43XX_GPIO_PIN(Pin);
    int f = 0;
    UINT8 ch;

    // Setup interrupt
    ch = GPIO_InitIRQ(Pin, ISR, ISR_Param);
    GPIO_SetIRQ(ch, IntEdge, TRUE);

    // Configure pin properties
    f = SCU_PINIO_FAST | ((port > 4) ? (4) : (0));
    PIN_Config(Pin, f);
    PIN_Mode(Pin, PullDown);
    port_reg->DIR[port] &= ~(1 << bit); // Intput

    return TRUE;
}

// ---------------------------------------------------------------------------
BOOL CPU_GPIO_GetPinState(GPIO_PIN Pin)
{
    LPC_GPIO_T *port_reg = (LPC_GPIO_T *) (LPC_GPIO_PORT_BASE);
    UINT8 port = LPC43XX_GPIO_PORT(Pin), bit = LPC43XX_GPIO_PIN(Pin);

    return ((port_reg->PIN[port] & (1 << bit)) ? 1 : 0);
}

// ---------------------------------------------------------------------------
void CPU_GPIO_SetPinState(GPIO_PIN Pin, BOOL PinState)
{
    LPC_GPIO_T *port_reg = (LPC_GPIO_T *) (LPC_GPIO_PORT_BASE);
    UINT8 port = LPC43XX_GPIO_PORT(Pin), bit = LPC43XX_GPIO_PIN(Pin);

    if (PinState)
        port_reg->SET[port] = (1 << bit);
    else
        port_reg->CLR[port] = (1 << bit);
}

// ---------------------------------------------------------------------------
BOOL CPU_GPIO_PinIsBusy(GPIO_PIN Pin)
{
    UINT8 port, bit;

    if (Pin > TOTAL_GPIO_PINS) { // Is Pin ID encoded?
        port = LPC43XX_GPIO_PORT(Pin); // If so, decode it
        bit = LPC43XX_GPIO_PIN(Pin);
    } else {
        port = Pin / 32;
        bit = Pin % 32;
    }

    return ((g_pinReserved[port] >> bit) & 1);
}

// ---------------------------------------------------------------------------
BOOL CPU_GPIO_ReservePin(GPIO_PIN Pin, BOOL fReserve)
{
    UINT8 port, bit;

    if (Pin > TOTAL_GPIO_PINS) { // Is Pin ID encoded?
        port = LPC43XX_GPIO_PORT(Pin); // If so, decode it
        bit = LPC43XX_GPIO_PIN(Pin);
    } else {
        port = Pin / 32;
        bit = Pin % 32;
    }

    if (fReserve)
    {
        if (g_pinReserved[port] & bit) return FALSE; // already reserved
        g_pinReserved[port] |= bit;
    } else {
        g_pinReserved[port] &= ~bit;
    }
    return TRUE;
}

// ---------------------------------------------------------------------------
UINT32 CPU_GPIO_GetDebounce()
{
    return (g_debounceTicks / (SLOW_CLOCKS_PER_SECOND / 1000)); // ticks -> ms
}

// ---------------------------------------------------------------------------
BOOL CPU_GPIO_SetDebounce(INT64 debounceTimeMilliseconds)
{
    if (debounceTimeMilliseconds > 0 && debounceTimeMilliseconds < 10000)
    {
        g_debounceTicks = CPU_MillisecondsToTicks((UINT32)debounceTimeMilliseconds);
        return TRUE;
    }
    return FALSE;
}

// ---------------------------------------------------------------------------
INT32 CPU_GPIO_GetPinCount()
{
    return TOTAL_GPIO_PINS;
} 

// ---------------------------------------------------------------------------
void CPU_GPIO_GetPinsMap(UINT8* pins, size_t size)
{
    for (int i = 0; i < size && i < TOTAL_GPIO_PINS; i++) {
         pins[i] = GPIO_ATTRIBUTE_INPUT | GPIO_ATTRIBUTE_OUTPUT;
    }
}

// ---------------------------------------------------------------------------
UINT8 CPU_GPIO_GetSupportedResistorModes(GPIO_PIN pin)
{
   return (1 << RESISTOR_DISABLED) | (1 << RESISTOR_PULLUP) | (1 << RESISTOR_PULLDOWN);
}

// ---------------------------------------------------------------------------
UINT8 CPU_GPIO_GetSupportedInterruptModes(GPIO_PIN pin)
{
    return (1 << GPIO_INT_EDGE_LOW) | (1 << GPIO_INT_EDGE_HIGH) | (1 << GPIO_INT_EDGE_BOTH)
         | (1 << GPIO_INT_LEVEL_LOW) | (1 << GPIO_INT_LEVEL_HIGH);
}
