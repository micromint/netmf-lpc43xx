////////////////////////////////////////////////////////////////////////////////
// LPC43XX_SPI.cpp - SPI functions for NXP LPC43XX
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

#define TOTAL_SPI_PORT  2

typedef struct
{
  LPC_SSP_T *reg;
  GPIO_PIN msk;
  GPIO_PIN miso;
  GPIO_PIN mosi;
} SPI_PORT_T;

static SPI_PORT_T const SPI_Port[TOTAL_SPI_PORT] = {
    {LPC_SSP0, (GPIO_PIN)P3_0, (GPIO_PIN)P1_1, (GPIO_PIN)P1_2},
    {LPC_SSP1, (GPIO_PIN)PF_4, (GPIO_PIN)P1_3, (GPIO_PIN)P1_4}};

#define SPI_REG(x)       (SPI_Port[x].reg)
#define SPI_MSK(x)       (SPI_Port[x].msk)
#define SPI_MISO(x)      (SPI_Port[x].miso)
#define SPI_MOSI(x)      (SPI_Port[x].mosi)

// Local functions
static BOOL SPI_Config(LPC_SSP_T *spi, int Bits, int Mode, int Slave);
static BOOL SPI_Frequency(LPC_SSP_T *spi, int Hz);
static inline int SPI_Enable(LPC_SSP_T *spi);
static inline int SPI_Disable(LPC_SSP_T *spi);
static inline int SPI_Read(LPC_SSP_T *spi);
static inline void SPI_Write(LPC_SSP_T *spi, int value);
static inline int SPI_Readable(LPC_SSP_T *spi);
static inline int SPI_Busy(LPC_SSP_T *spi);

static BOOL SPI_Config(LPC_SSP_T *spi, int Bits, int Mode, int Slave)
{
    SPI_Disable(spi);
    
    if (!(Bits >= 4 && Bits <= 16) || !(Mode >= 0 && Mode <= 3)) {
        return FALSE; // SPI format error
    }
    
    int polarity = (Mode & 0x2) ? 1 : 0;
    int phase = (Mode & 0x1) ? 1 : 0;
    
    // set it up
    int DSS = Bits - 1;            // DSS (data select size)
    int SPO = (polarity) ? 1 : 0;  // SPO - clock out polarity
    int SPH = (phase) ? 1 : 0;     // SPH - clock out phase
    
    int FRF = 0;                   // FRF (frame format) = SPI
    uint32_t tmp = spi->CR0;
    tmp &= ~(0xFFFF);
    tmp |= DSS << 0
        | FRF << 4
        | SPO << 6
        | SPH << 7;
    spi->CR0 = tmp;
    
    tmp = spi->CR1;
    tmp &= ~(0xD);
    tmp |= 0 << 0                  // LBM - loop back mode - off
        | ((Slave) ? 1 : 0) << 2   // MS  - master Slave mode, 1 = Slave
        | 0 << 3;                  // SOD - slave output disable - na
    spi->CR1 = tmp;
    SPI_Enable(spi);
    return TRUE;
}

static BOOL SPI_Frequency(LPC_SSP_T *spi, int Hz)
{
    SPI_Disable(spi);
    
    uint32_t PCLK = SystemCoreClock;
    
    int prescaler;
    
    for (prescaler = 2; prescaler <= 254; prescaler += 2) {
        int prescale_Hz = PCLK / prescaler;
        
        // calculate the divider
        int divider = floor(((float)prescale_Hz / (float)Hz) + 0.5f);
        
        // check we can support the divider
        if (divider < 256) {
            // prescaler
            spi->CPSR = prescaler;
            
            // divider
            spi->CR0 &= ~(0xFFFF << 8);
            spi->CR0 |= (divider - 1) << 8;
            SPI_Enable(spi);
            return TRUE;
        }
    }
    return FALSE; // Couldn't setup requested SPI frequency
}

static inline int SPI_Enable(LPC_SSP_T *spi)
{
    return spi->CR1 |= (1 << 1);
}

static inline int SPI_Disable(LPC_SSP_T *spi)
{
    return spi->CR1 &= ~(1 << 1);
}


static inline int SPI_Read(LPC_SSP_T *spi)
{
    return spi->DR;
}

static inline void SPI_Write(LPC_SSP_T *spi, int value)
{
    spi->DR = value;
}

static inline int SPI_Readable(LPC_SSP_T *spi)
{
    return spi->SR & (1 << 2);
}

static inline int SPI_Busy(LPC_SSP_T *spi)
{
    return (spi->SR & (1 << 4)) ? (1) : (0);
}

BOOL CPU_SPI_Initialize()
{
    return TRUE;
}

void CPU_SPI_Uninitialize()
{
}

BOOL CPU_SPI_nWrite16_nRead16(const SPI_CONFIGURATION& Configuration, UINT16* Write16, INT32 WriteCount, UINT16* Read16, INT32 ReadCount, INT32 ReadStartOffset)
{
    GLOBAL_LOCK(irq);

    if(!CPU_SPI_Xaction_Start( Configuration )) return FALSE;

    SPI_XACTION_16 Transaction;
    Transaction.Read16          = Read16;
    Transaction.ReadCount       = ReadCount;
    Transaction.ReadStartOffset = ReadStartOffset;
    Transaction.Write16         = Write16;
    Transaction.WriteCount      = WriteCount;
    Transaction.SPI_mod         = Configuration.SPI_mod;

    if(!CPU_SPI_Xaction_nWrite16_nRead16( Transaction )) return FALSE;
    return CPU_SPI_Xaction_Stop( Configuration );
}

BOOL CPU_SPI_nWrite8_nRead8(const SPI_CONFIGURATION& Configuration, UINT8* Write8, INT32 WriteCount, UINT8* Read8, INT32 ReadCount, INT32 ReadStartOffset)
{
    GLOBAL_LOCK(irq);

    if(!CPU_SPI_Xaction_Start( Configuration )) return FALSE;

    SPI_XACTION_8 Transaction;
    Transaction.Read8           = Read8;
    Transaction.ReadCount       = ReadCount;
    Transaction.ReadStartOffset = ReadStartOffset;
    Transaction.Write8          = Write8;
    Transaction.WriteCount      = WriteCount;
    Transaction.SPI_mod         = Configuration.SPI_mod;

    if(!CPU_SPI_Xaction_nWrite8_nRead8( Transaction )) return FALSE;
    return CPU_SPI_Xaction_Stop( Configuration );
}

BOOL CPU_SPI_Xaction_Start(const SPI_CONFIGURATION& Configuration)
{
    if (Configuration.SPI_mod >= TOTAL_SPI_PORT) return FALSE;

    LPC_SSP_T *spi = SPI_REG(Configuration.SPI_mod);
    int Bits, Mode;

    // Configure options and clock
    Bits = (Configuration.MD_16bits) ? 16 : 8;
    Mode = (Configuration.MSK_IDLE) ? 2 : 0; // ToDo: Check
    Mode |= (!Configuration.MSK_SampleEdge) ? 1 : 0;
    SPI_Config(spi, Bits, Mode, 0);
    SPI_Frequency(spi, (1000 * Configuration.Clock_RateKHz));
    
    // I/O setup
    GPIO_PIN msk, miso, mosi;
    CPU_SPI_GetPins(Configuration.SPI_mod, msk, miso, mosi);
    UINT32 alternate = 0x252; // AF5 = SPI1/SPI2
    if (Configuration.SPI_mod == 2) alternate = 0x262; // AF6 = SPI3, speed = 2 (50MHz)
    CPU_GPIO_DisablePin(msk,  RESISTOR_DISABLED, 1, (GPIO_ALT_MODE)alternate);
    CPU_GPIO_DisablePin(miso, RESISTOR_DISABLED, 0, (GPIO_ALT_MODE)alternate);
    CPU_GPIO_DisablePin(mosi, RESISTOR_DISABLED, 1, (GPIO_ALT_MODE)alternate);

    // CS setup
    CPU_GPIO_EnableOutputPin(Configuration.DeviceCS, Configuration.CS_Active);
    if(Configuration.CS_Setup_uSecs)
    {
        HAL_Time_Sleep_MicroSeconds_InterruptEnabled(Configuration.CS_Setup_uSecs);
    }

    return TRUE;
}

BOOL CPU_SPI_Xaction_Stop(const SPI_CONFIGURATION& Configuration)
{
    LPC_SSP_T *spi = SPI_REG(Configuration.SPI_mod);
    while (SPI_Busy(spi)); // wait for completion

    if(Configuration.CS_Hold_uSecs)
    {
        HAL_Time_Sleep_MicroSeconds_InterruptEnabled(Configuration.CS_Hold_uSecs);
    }
    CPU_GPIO_SetPinState(Configuration.DeviceCS, !Configuration.CS_Active);
    GPIO_RESISTOR res = RESISTOR_PULLDOWN;
    if (Configuration.MSK_IDLE) res = RESISTOR_PULLUP;
    GPIO_PIN msk, miso, mosi;
    CPU_SPI_GetPins(Configuration.SPI_mod, msk, miso, mosi);
    CPU_GPIO_EnableInputPin(msk,  FALSE, NULL, GPIO_INT_NONE, res);
    CPU_GPIO_EnableInputPin(miso, FALSE, NULL, GPIO_INT_NONE, RESISTOR_PULLDOWN);
    CPU_GPIO_EnableInputPin(mosi, FALSE, NULL, GPIO_INT_NONE, RESISTOR_PULLDOWN);

    SPI_Disable(spi); // Disable SPI
    return TRUE;
}

BOOL CPU_SPI_Xaction_nWrite16_nRead16(SPI_XACTION_16& Transaction)
{
    LPC_SSP_T *spi = SPI_REG(Transaction.SPI_mod);
    
    UINT16* outBuf = Transaction.Write16;
    UINT16* inBuf  = Transaction.Read16;
    INT32 outLen = Transaction.WriteCount;
    INT32 num, ii, i = 0;
    
    if (Transaction.ReadCount) { // write & read
        num = Transaction.ReadCount + Transaction.ReadStartOffset;
        ii = -Transaction.ReadStartOffset;
    } else { // write only
        num = outLen;
        ii = 0x80000000; // disable write to inBuf
    }

    UINT16 out = outBuf[0];
    UINT16 in;
    SPI_Write(spi, out); // write first word
    while (++i < num) {
        if (i < outLen) out = outBuf[i]; // get new output data
        while (!(SPI_Readable(spi))); // wait for Rx buffer
        in = SPI_Read(spi); // read input
        SPI_Write(spi, out); // start output
        if (ii >= 0) inBuf[ii] = in; // save input data
        ii++;
    }
    while (!(SPI_Readable(spi))); // wait for Rx buffer
    in = SPI_Read(spi); // read last input
    if (ii >= 0) inBuf[ii] = in; // save last input

    return TRUE;
}

BOOL CPU_SPI_Xaction_nWrite8_nRead8( SPI_XACTION_8& Transaction )
{
    LPC_SSP_T *spi = SPI_REG(Transaction.SPI_mod);
    
    UINT8* outBuf = Transaction.Write8;
    UINT8* inBuf  = Transaction.Read8;
    INT32 outLen = Transaction.WriteCount;
    INT32 num, ii, i = 0;
    
    if (Transaction.ReadCount) { // write & read
        num = Transaction.ReadCount + Transaction.ReadStartOffset;
        ii = -Transaction.ReadStartOffset;
    } else { // write only
        num = outLen;
        ii = 0x80000000; // disable write to inBuf
    }

    UINT8 out = outBuf[0];
    UINT16 in;
    SPI_Write(spi, out); // write first word
    while (++i < num) {
        if (i < outLen) out = outBuf[i]; // get new output data
        while (!(SPI_Readable(spi))); // wait for Rx buffer
        in = SPI_Read(spi); // read input
        SPI_Write(spi, out); // start output
        if (ii >= 0) inBuf[ii] = (UINT8)in; // save input data
        ii++;
    }
    while (!(SPI_Readable(spi))); // wait for Rx buffer
    in = SPI_Read(spi); // read last input
    if (ii >= 0) inBuf[ii] = (UINT8)in; // save last input

    return TRUE;
}

UINT32 CPU_SPI_PortsCount()
{
    return TOTAL_SPI_PORT;
}

void CPU_SPI_GetPins(UINT32 spi_mod, GPIO_PIN& msk, GPIO_PIN& miso, GPIO_PIN& mosi)
{
    if (spi_mod >= TOTAL_SPI_PORT)
    {
        msk  = GPIO_PIN_NONE;
        miso = GPIO_PIN_NONE;
        mosi = GPIO_PIN_NONE;
    } else {
        msk  = SPI_MSK(spi_mod);
        miso = SPI_MISO(spi_mod);
        mosi = SPI_MOSI(spi_mod);
    }

    return;
}
