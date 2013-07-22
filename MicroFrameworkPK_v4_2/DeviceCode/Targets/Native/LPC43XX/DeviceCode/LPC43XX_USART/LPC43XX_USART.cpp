////////////////////////////////////////////////////////////////////////////////
// LPC43XX_USART.cpp - USART functions for NXP LPC43XX
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

// USART interrupt handlers
void USART0_IRQHandler(void* param);
void USART1_IRQHandler(void* param);
void USART2_IRQHandler(void* param);
void USART3_IRQHandler(void* param);

typedef struct
{
  LPC_USART_T *reg;
  GPIO_PIN txPin;
  int txConf;
  GPIO_PIN rxPin;
  int rxConf;
  UINT32 irq;
  HAL_CALLBACK_FPN isr;
} USART_PORT_T;

static USART_PORT_T const USART_Port[TOTAL_USART_PORT] = {
    {LPC_USART0, (GPIO_PIN)UART0_TX, 2, (GPIO_PIN)UART0_RX, 2, USART0_IRQn, USART0_IRQHandler},
    {LPC_UART1,  (GPIO_PIN)UART1_TX, 4, (GPIO_PIN)UART1_RX, 1, UART1_IRQn,  USART1_IRQHandler},
    {LPC_USART2, (GPIO_PIN)UART2_TX, 2, (GPIO_PIN)UART2_RX, 2, USART2_IRQn, USART2_IRQHandler},
    {LPC_USART3, (GPIO_PIN)UART3_TX, 2, (GPIO_PIN)UART3_RX, 2, USART3_IRQn, USART3_IRQHandler}};

#define USART_REG(x)     (USART_Port[x].reg)
#define USART_TxPin(x)   (USART_Port[x].txPin)
#define USART_TxConf(x)  (USART_Port[x].txConf)
#define USART_RxPin(x)   (USART_Port[x].rxPin)
#define USART_RxConf(x)  (USART_Port[x].rxConf)
#define USART_IRQ(x)     (USART_Port[x].irq)
#define USART_ISR(x)     (USART_Port[x].isr)

// Local functions
static BOOL USART_Config(int ComPortNum, int BaudRate, int Parity,
                         int DataBits, int StopBits, int FlowValue);

// ---------------------------------------------------------------------------
static BOOL USART_Config(int ComPortNum, int BaudRate, int Parity,
                  int DataBits, int StopBits, int FlowValue)
{
    LPC_USART_T *usart = USART_REG(ComPortNum);
    uint32_t PCLK = SystemCoreClock;

    // First we check to see if the basic divide with no DivAddVal/MulVal
    // ratio gives us an integer result. If it doesG// and/or lookup tables, but the brute force method is not that much
    // slower, and is more maintainable.
    uint16_t DL = PCLK / (16 * BaudRate);

    uint8_t DivAddVal = 0;
    uint8_t MulVal = 1;
    int hit = 0;
    uint16_t dlv;
    uint8_t mv, dav;
    if ((PCLK % (16 * BaudRate)) != 0) {     // Checking for zero remainder
        float err_best = (float) BaudRate;
        uint16_t dlmax = DL;
        for ( dlv = (dlmax/2); (dlv <= dlmax) && !hit; dlv++) {
            for ( mv = 1; mv <= 15; mv++) {
                for ( dav = 1; dav < mv; dav++) {
                    float ratio = 1.0f + ((float) dav / (float) mv);
                    float calcbaud = (float)PCLK / (16.0f * (float) dlv * ratio);
                    float err = fabs(((float) BaudRate - calcbaud) / (float) BaudRate);
                    if (err < err_best) {
                        DL = dlv;
                        DivAddVal = dav;
                        MulVal = mv;
                        err_best = err;
                        if (err < 0.001f) {
                            hit = 1;
                        }
                    }
                }
            }
        }
    }
    
    // set LCR[DLAB] to enable writing to divider registers
    usart->LCR |= (1 << 7);
    
    // set divider values
    usart->DLM = (DL >> 8) & 0xFF;
    usart->DLL = (DL >> 0) & 0xFF;
    usart->FDR = (uint32_t) DivAddVal << 0
                   | (uint32_t) MulVal    << 4;
    
    // clear LCR[DLAB]
    usart->LCR &= ~(1 << 7);

    // Set serial format
    // 0: 1 stop bits, 1: 2 stop bits
    if (StopBits != 1 && StopBits != 2) {
        return FALSE; // Invalid stop bits specified
    }
    StopBits -= 1;
    
    // 0: 5 data bits ... 3: 8 data bits
    if (DataBits < 5 || DataBits > 8) {
        return FALSE; // Invalid number of bits (%d) in serial format, should be 5..8
    }
    DataBits -= 5;

    int Parity_enable, Parity_select;
    switch (Parity) {
        case USART_PARITY_NONE: Parity_enable = 0; Parity_select = 0; break;
        case USART_PARITY_ODD: Parity_enable = 1; Parity_select = 0; break;
        case USART_PARITY_EVEN: Parity_enable = 1; Parity_select = 1; break;
        case USART_PARITY_MARK: Parity_enable = 1; Parity_select = 2; break;
        case USART_PARITY_SPACE: Parity_enable = 1; Parity_select = 3; break;
        default:
            return FALSE; // Invalid serial Parity setting
    }
    
    usart->LCR = (DataBits << 0) | (StopBits << 2)
                 | (Parity_enable << 3) | (Parity_select << 4);
    return TRUE;
}

// ---------------------------------------------------------------------------
void USART_IRQHandler(int ComPortNum, int Direction)
{
    LPC_USART_T *usart = USART_REG(ComPortNum);
    char c;

    switch (Direction)
    {
        case 1: // Transmit
            if (USART_RemoveCharFromTxBuffer(ComPortNum, c))
            {
                usart->THR = c;  // Write data
            } else {
                CPU_USART_TxBufferEmptyInterruptEnable(ComPortNum, FALSE); // Disable interrupt
            }
            Events_Set(SYSTEM_EVENT_FLAG_COM_OUT);
            break;

        case 2: // Receive
            c = (char)(usart->RBR); // Read data
            USART_AddCharToRxBuffer(ComPortNum, c);
            Events_Set(SYSTEM_EVENT_FLAG_COM_IN);
            break;

        default:
            break;
    }
}

// ---------------------------------------------------------------------------
void USART0_IRQHandler(void* param)
{
    USART_IRQHandler(0, (LPC_USART0->IIR >> 1) & 0x7);
}

// ---------------------------------------------------------------------------
void USART1_IRQHandler(void* param)
{
    USART_IRQHandler(1, (LPC_UART1->IIR >> 1) & 0x7);
}

// ---------------------------------------------------------------------------
void USART2_IRQHandler(void* param)
{
    USART_IRQHandler(2, (LPC_USART2->IIR >> 1) & 0x7);
}

// ---------------------------------------------------------------------------
void USART3_IRQHandler(void* param)
{
    USART_IRQHandler(3, (LPC_USART3->IIR >> 1) & 0x7);
}

// ---------------------------------------------------------------------------
BOOL CPU_USART_Initialize(int ComPortNum, int BaudRate, int Parity,
                          int DataBits, int StopBits, int FlowValue)
{
    LPC_USART_T *usart = USART_REG(ComPortNum);

    if (ComPortNum >= TOTAL_USART_PORT) return FALSE;

    GLOBAL_LOCK(irq);

    // Enable fifos and default rx trigger level
    usart->FCR = 1 << 0  // FIFO Enable - 0 = Disables, 1 = Enabled
               | 0 << 1  // Rx Fifo Reset
               | 0 << 2  // Tx Fifo Reset
               | 0 << 6; // Rx irq trigger level - 0 = 1 char, 1 = 4 chars, 2 = 8 chars, 3 = 14 chars

    // Disable irqs
    usart->IER = 0 << 0  // Rx Data available irq enable
               | 0 << 1  // Tx Fifo empty irq enable
               | 0 << 2; // Rx Line Status irq enable
    
    // Configure baud rate and format
    USART_Config(ComPortNum, BaudRate, Parity, DataBits, StopBits, FlowValue);
    
    // Configure USART pins
    PIN_Config(USART_TxPin(ComPortNum), (SCU_PINIO_PULLUP | USART_TxConf(ComPortNum)));
    PIN_Config(USART_RxPin(ComPortNum), (SCU_PINIO_PULLUP | USART_RxConf(ComPortNum)));

    // Unprotect pins and activate interrupts
    CPU_USART_ProtectPins(ComPortNum, FALSE);
    CPU_INTC_ActivateInterrupt(USART_IRQ(ComPortNum), USART_ISR(ComPortNum), 0);

    return TRUE;
}

// ---------------------------------------------------------------------------
BOOL CPU_USART_Uninitialize(int ComPortNum)
{
    LPC_USART_T *usart = USART_REG(ComPortNum);

    GLOBAL_LOCK(irq);

    usart->FCR = 1 << 1  // Rx FIFO reset
               | 1 << 2  // Tx FIFO reset
               | 0 << 6; // Interrupt depth

    CPU_INTC_DeactivateInterrupt(USART_IRQ(ComPortNum));
    CPU_USART_ProtectPins(ComPortNum, TRUE);  
    return TRUE;
}

// ---------------------------------------------------------------------------
BOOL CPU_USART_TxBufferEmpty(int ComPortNum)
{
    return ((USART_REG(ComPortNum)->LSR & (1 << 5)) ? TRUE : FALSE);
}

// ---------------------------------------------------------------------------
BOOL CPU_USART_TxShiftRegisterEmpty(int ComPortNum)
{
    return ((USART_REG(ComPortNum)->LSR & (1 << 6)) ? TRUE : FALSE);
}

// ---------------------------------------------------------------------------
void CPU_USART_WriteCharToTxBuffer(int ComPortNum, UINT8 c)
{
    USART_REG(ComPortNum)->THR = c;
}

// ---------------------------------------------------------------------------
void CPU_USART_TxBufferEmptyInterruptEnable(int ComPortNum, BOOL Enable)
{
    LPC_USART_T *usart = USART_REG(ComPortNum);

    if (Enable)
    {
        usart->IER |=  0x02;
    }
    else
    {
        usart->IER &= ~(0x02);
    }
}

// ---------------------------------------------------------------------------
BOOL CPU_USART_TxBufferEmptyInterruptState(int ComPortNum)
{
    if (USART_REG(ComPortNum)->IER & 0x02) return TRUE;
    return FALSE;
}

// ---------------------------------------------------------------------------
void CPU_USART_RxBufferFullInterruptEnable(int ComPortNum, BOOL Enable)
{
    LPC_USART_T *usart = USART_REG(ComPortNum);

    if (Enable)
    {
       usart->IER |=  0x01;
    }
    else
    {
       usart->IER &= ~(0x01);
    }
}

// ---------------------------------------------------------------------------
BOOL CPU_USART_RxBufferFullInterruptState(int ComPortNum)
{
    if (USART_REG(ComPortNum)->IER & 0x01) return TRUE;
    return FALSE;
}

// ---------------------------------------------------------------------------
void CPU_USART_ProtectPins(int ComPortNum, BOOL On)
{
    if (On)
    {
        CPU_USART_RxBufferFullInterruptEnable(ComPortNum, FALSE);
        CPU_USART_TxBufferEmptyInterruptEnable(ComPortNum, FALSE);
    } else {
        CPU_USART_TxBufferEmptyInterruptEnable(ComPortNum, TRUE);
        CPU_USART_RxBufferFullInterruptEnable(ComPortNum, TRUE);
    }
}

// ---------------------------------------------------------------------------
UINT32 CPU_USART_PortsCount()
{
    return TOTAL_USART_PORT;
}

// ---------------------------------------------------------------------------
void CPU_USART_GetPins( int ComPortNum, GPIO_PIN& rxPin, GPIO_PIN& txPin,GPIO_PIN& ctsPin, GPIO_PIN& rtsPin )
{   
    rxPin = USART_RxPin(ComPortNum); 
    txPin = USART_TxPin(ComPortNum); 
    ctsPin= GPIO_PIN_NONE; 
    rtsPin= GPIO_PIN_NONE; 

    return;
}

// ---------------------------------------------------------------------------
BOOL CPU_USART_SupportNonStandardBaudRate(int ComPortNum)
{
    return TRUE;
}

// ---------------------------------------------------------------------------
void CPU_USART_GetBaudrateBoundary(int ComPortNum, UINT32& maxBaudRateHz, UINT32& minBaudRateHz)
{
    uint32_t PCLK = SystemCoreClock;

    maxBaudRateHz = PCLK >> 4;
    minBaudRateHz = 75;
}

// ---------------------------------------------------------------------------
BOOL CPU_USART_IsBaudrateSupported(int ComPortNum, UINT32& BaudrateHz)
{
    UINT32 maxBaudRateHz, minBaudRateHz;

    CPU_USART_GetBaudrateBoundary(ComPortNum, maxBaudRateHz, minBaudRateHz);

    if (BaudrateHz <= maxBaudRateHz) return TRUE;
    BaudrateHz = maxBaudRateHz;
    return FALSE;
}

// ---------------------------------------------------------------------------
BOOL CPU_USART_TxHandshakeEnabledState(int comPort)
{
    return TRUE;
}
