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

// Registers on LPC43XX USARTs to 550 industry standard (16C550)

// USART interrupt handlers
void USART_IRQHandler(int ComPortNum);
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
  GPIO_PIN rtsPin;
  int rtsConf;
  GPIO_PIN ctsPin;
  int ctsConf;
  UINT32 irq;
  HAL_CALLBACK_FPN isr;
} USART_PORT_T;

static USART_PORT_T const __section(rodata) USART_Port[TOTAL_USART_PORT] = {
    {LPC_USART0, (GPIO_PIN)UART0_TX, 2, (GPIO_PIN)UART0_RX, 2, 0, 0, 0, 0, USART0_IRQn, USART0_IRQHandler},
    {LPC_UART1,  (GPIO_PIN)UART1_TX, 4, (GPIO_PIN)UART1_RX, 1,
                 (GPIO_PIN)UART1_RTS, 4, (GPIO_PIN)UART1_CTS, 4, UART1_IRQn,  USART1_IRQHandler},
    {LPC_USART2, (GPIO_PIN)UART2_TX, 2, (GPIO_PIN)UART2_RX, 2, 0, 0, 0, 0, USART2_IRQn, USART2_IRQHandler},
    {LPC_USART3, (GPIO_PIN)UART3_TX, 2, (GPIO_PIN)UART3_RX, 2, 0, 0, 0, 0, USART3_IRQn, USART3_IRQHandler}};

#define USART_REG(x)     (USART_Port[x].reg)
#define USART_TxPin(x)   (USART_Port[x].txPin)
#define USART_TxConf(x)  (USART_Port[x].txConf)
#define USART_RxPin(x)   (USART_Port[x].rxPin)
#define USART_RxConf(x)  (USART_Port[x].rxConf)
#define USART_RtsPin(x)  (USART_Port[x].rtsPin)
#define USART_RtsConf(x) (USART_Port[x].rtsConf)
#define USART_CtsPin(x)  (USART_Port[x].ctsPin)
#define USART_CtsConf(x) (USART_Port[x].ctsConf)
#define USART_IRQ(x)     (USART_Port[x].irq)
#define USART_ISR(x)     (USART_Port[x].isr)

// USART IER (Interrupt Enable Register) Flags
#define IER_RxIrq        (1 << 0)
#define IER_TxIrq        (1 << 1)
// USART IIR (Interrupt Identification Register) Flags
#define IIR_NoInt        (1 << 0)
// USART LSR (Line Status Register) Flags
#define LSR_RxBufData    (1 << 0)
#define LSR_TxBufEmpty   (1 << 5)
#define LSR_TxRegEmpty   (1 << 6)

// Local functions
static BOOL USART_Config(int ComPortNum, int BaudRate, int Parity,
                         int DataBits, int StopBits, int FlowValue);

// ---------------------------------------------------------------------------
void USART_IRQHandler(int ComPortNum)
{
    LPC_USART_T *usart = USART_REG(ComPortNum);
    char c;

    while (!(usart->IIR & IIR_NoInt)) // Interrupt pending?
    {
        int lsr = usart->LSR;
        if (lsr & LSR_TxBufEmpty) // Transmitter available?
        {
            if (USART_RemoveCharFromTxBuffer(ComPortNum, c))
            {
                usart->THR = c;  // Write data
                Events_Set(SYSTEM_EVENT_FLAG_COM_OUT);
            } else {
                CPU_USART_TxBufferEmptyInterruptEnable(ComPortNum, FALSE); // Disable interrupt
            }
        }
        if (lsr & LSR_RxBufData) // Data received?
        {
            c = (char)(usart->RBR); // Read data
            USART_AddCharToRxBuffer(ComPortNum, c);
            Events_Set(SYSTEM_EVENT_FLAG_COM_IN);
        }
    }
}

// ---------------------------------------------------------------------------
// Need multiple ISRs since param is ignored
void USART0_IRQHandler(void* param) { USART_IRQHandler(0); }
void USART1_IRQHandler(void* param) { USART_IRQHandler(1); }
void USART2_IRQHandler(void* param) { USART_IRQHandler(2); }
void USART3_IRQHandler(void* param) { USART_IRQHandler(3); }

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
BOOL CPU_USART_Initialize(int ComPortNum, int BaudRate, int Parity,
                          int DataBits, int StopBits, int FlowValue)
{
    LPC_USART_T *usart = USART_REG(ComPortNum);
    volatile int tmp;

    if (ComPortNum >= TOTAL_USART_PORT) return FALSE;

    GLOBAL_LOCK(irq);

    // Configure USART pins
    PIN_Config(USART_TxPin(ComPortNum), (SCU_MODE_MODE_REPEATER | USART_TxConf(ComPortNum)));
    PIN_Config(USART_RxPin(ComPortNum), (SCU_PINIO_PULLNONE | USART_RxConf(ComPortNum)));

    // Configure baud rate and format, enable clock
    usart->LCR = 0;
  	usart->ACR = 0;
    USART_Config(ComPortNum, BaudRate, Parity, DataBits, StopBits, FlowValue);
    LPC_CGU->BASE_CLK[(CLK_BASE_UART0 + ComPortNum)] &= ~1;

    // Enable transmitter, set modem flow control if COM2
    usart->TER1 = (1 << 7);
    if (ComPortNum == 1) {
        	usart->MCR = 0; // (1 << 6) | (1 << 7); // Auto RTS/CTS
            tmp = usart->MSR; // Clear status
    } else {
            usart->TER2 = (1 << 0);
    }

    // Enable and reset FIFOs, level 2 = 8 char
    usart->FCR = (1 << 0) | (1 << 1) | (1 << 2) | (2 << 6);

    // Unprotect pins and activate interrupts
    usart->IER = 0; // Clear interrupts
    CPU_USART_ProtectPins(ComPortNum, FALSE);
    CPU_INTC_ActivateInterrupt(USART_IRQ(ComPortNum), USART_ISR(ComPortNum), 0);

    return TRUE;
}

// ---------------------------------------------------------------------------
BOOL CPU_USART_Uninitialize(int ComPortNum)
{
    LPC_USART_T *usart = USART_REG(ComPortNum);

    GLOBAL_LOCK(irq);

    // Disable FIFOs, clock and interrupts
    usart->FCR = 0;  // Disable FIFOs
    LPC_CGU->BASE_CLK[(CLK_BASE_UART0 + ComPortNum)] |= 1;
    CPU_INTC_DeactivateInterrupt(USART_IRQ(ComPortNum));
    CPU_USART_ProtectPins(ComPortNum, TRUE);  
    return TRUE;
}

// ---------------------------------------------------------------------------
BOOL CPU_USART_TxBufferEmpty(int ComPortNum)
{
    return ((USART_REG(ComPortNum)->LSR & LSR_TxBufEmpty) ? TRUE : FALSE);
}

// ---------------------------------------------------------------------------
BOOL CPU_USART_TxShiftRegisterEmpty(int ComPortNum)
{
    return ((USART_REG(ComPortNum)->LSR & LSR_TxRegEmpty) ? TRUE : FALSE);
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
        usart->IER |=  (IER_TxIrq);

        // Tx Empty irq in USART will trigger after data is transmitted.
        // If transmitter available and data is pending, get it started
        char c;
        if (usart->LSR & LSR_TxBufEmpty && USART_RemoveCharFromTxBuffer(ComPortNum, c))
        {
            usart->THR = c;  // Write data
            Events_Set(SYSTEM_EVENT_FLAG_COM_OUT);
        }
    }
    else
    {
        usart->IER &= ~(IER_TxIrq);
    }
}

// ---------------------------------------------------------------------------
BOOL CPU_USART_TxBufferEmptyInterruptState(int ComPortNum)
{
    if (USART_REG(ComPortNum)->IER & IER_TxIrq) return TRUE;
    return FALSE;
}

// ---------------------------------------------------------------------------
void CPU_USART_RxBufferFullInterruptEnable(int ComPortNum, BOOL Enable)
{
    LPC_USART_T *usart = USART_REG(ComPortNum);

    if (Enable)
    {
       usart->IER |=  IER_RxIrq;
    }
    else
    {
       usart->IER &= ~(IER_RxIrq);
    }
}

// ---------------------------------------------------------------------------
BOOL CPU_USART_RxBufferFullInterruptState(int ComPortNum)
{
    if (USART_REG(ComPortNum)->IER & IER_RxIrq) return TRUE;
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
    ctsPin= (USART_CtsPin(ComPortNum)) ? USART_CtsPin(ComPortNum) : GPIO_PIN_NONE; 
    rtsPin= (USART_RtsPin(ComPortNum)) ? USART_RtsPin(ComPortNum) : GPIO_PIN_NONE;

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
