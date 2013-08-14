////////////////////////////////////////////////////////////////////////////////
// LPC43XX_Bootstrap.cpp - Bootstrap functions for NXP LPC43XX
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
// Copyright (c) NXP Semiconductors.  All rights reserved.
// Copyright (c) Microsoft Corporation.  All rights reserved.
// Ported to NXP LPC43XX by Micromint USA <support@micromint.com>
////////////////////////////////////////////////////////////////////////////////

#include <tinyhal.h>
#include "LPC43XX.h"

#define COUNT_OF(a) (sizeof(a)/sizeof(a[0]))

void BootstrapCode_GPIO();

#if !defined(CORE_M0)
/* SCU pin definitions for pin muxing */
typedef struct {
    __IO uint32_t *reg; /* SCU register address */
    uint16_t mode;      /* SCU pin mode and function */
} PINMUX_GRP_T;

/* Local functions */
void SystemInit(void);
static void SystemSetupClock(void);
static void SystemSetupPins(const PINMUX_GRP_T *mux, uint32_t n);
static void SystemSetupMemory(void);
static void WaitUs(uint32_t us);

/* Pins to initialize before clocks are configured */
static const PINMUX_GRP_T pre_clock_mux[] = {
    /* SPIFI pins */
    {SCU_REG(0x3, 3), (SCU_PINIO_FAST | 0x3)},  // P3_3 SPIFI CLK
    {SCU_REG(0x3, 4), (SCU_PINIO_FAST | 0x3)},  // P3_4 SPIFI D3
    {SCU_REG(0x3, 5), (SCU_PINIO_FAST | 0x3)},  // P3_5 SPIFI D2
    {SCU_REG(0x3, 6), (SCU_PINIO_FAST | 0x3)},  // P3_6 SPIFI D1
    {SCU_REG(0x3, 7), (SCU_PINIO_FAST | 0x3)},  // P3_7 SPIFI D0
    {SCU_REG(0x3, 8), (SCU_PINIO_FAST | 0x3)}   // P3_8 SPIFI CS/SSEL
};

/* Pins to initialize after clocks are configured */
static const PINMUX_GRP_T post_clock_mux[] = {
    /* Boot pins */
    {SCU_REG(0x1, 1), (SCU_PINIO_FAST | 0x0)},  // P1_1  BOOT0
    {SCU_REG(0x1, 2), (SCU_PINIO_FAST | 0x0)},  // P1_2  BOOT1
    {SCU_REG(0x2, 8), (SCU_PINIO_FAST | 0x0)},  // P2_8  BOOT2
    {SCU_REG(0x2, 9), (SCU_PINIO_FAST | 0x0)},  // P2_9  BOOT3
    /* Micromint Bambino 200 */
    {SCU_REG(0x6, 11), (SCU_PINIO_FAST | 0x0)}, // P6_11 LED1
    {SCU_REG(0x2, 5), (SCU_PINIO_FAST | 0x0)},  // P2_5  LED2
    {SCU_REG(0x2, 7), (SCU_PINIO_FAST | 0x0)}   // P2_7  BTN1
};
#endif /* !defined(CORE_M0) */

/*
 * SystemInit() - Initialize the system
 */
void __section(SectionForBootstrapOperations) SystemInit(void)
{
#if !defined(CORE_M0)
    unsigned int *pSCB_VTOR = (unsigned int *) 0xE000ED08;

#if defined(__ARMCC_VERSION)
    extern void *__Vectors;

    *pSCB_VTOR = (unsigned int) &__Vectors;
#elif defined(__IAR_SYSTEMS_ICC__)
    extern void *__vector_table;

    *pSCB_VTOR = (unsigned int) &__vector_table;
#else /* defined(__GNUC__) and others */
    extern void *g_pfnVectors;

    *pSCB_VTOR = (unsigned int) &g_pfnVectors;
#endif

#if 0 // defined(__FPU_PRESENT) && __FPU_PRESENT == 1
    /* Initialize floating point */
    fpuInit();
#endif

    SystemSetupPins(pre_clock_mux, COUNT_OF(pre_clock_mux)); /* Configure pins */
    SystemSetupClock();   /* Configure processor and peripheral clocks */
    SystemSetupPins(post_clock_mux, COUNT_OF(post_clock_mux)); /* Configure pins */
    SystemSetupMemory();  /* Configure external memory */
#endif /* !defined(CORE_M0) */
}

#if !defined(CORE_M0)
/*
 * SystemSetupClock() - Set processor and peripheral clocks
 *
 *    Clock       Frequency    Source
 * CLK_BASE_MX     204 MHz    CLKIN_PLL1
 * CLK_BASE_SPIFI  102 MHz    CLKIN_IDIVE
 * CLK_BASE_USB0   480 MHz    CLKIN_PLL0USB (Disabled)
 * CLK_BASE_USB1    60 MHz    CLKIN_IDIVE  (Disabled)
 *                 120 MHz    CLKIN_IDIVD  (Disabled)
 *
 *                  12 MHz    CLKIN_IDIVB
 *                  12 MHz    CLKIN_IDIVC
 *
 */
void __section(SectionForBootstrapOperations) SystemSetupClock(void)
{
#if (CLOCK_SETUP)
    /* Switch main clock to Internal RC (IRC) while setting up PLL1 */
    LPC_CGU->BASE_CLK[CLK_BASE_MX] = (1 << 11) | (CLKIN_IRC << 24);

    /* Enable the oscillator and wait 100 us */
    LPC_CGU->XTAL_OSC_CTRL = 0;
    WaitUs(100);

#if (SPIFI_INIT)
    /* Setup SPIFI control register and no-opcode mode */
    LPC_SPIFI->CTRL = (0x100 << 0) | (1 << 16) | (1 << 29) | (1 << 30);
    LPC_SPIFI->IDATA = 0xA5;
    /* Switch IDIVE clock to IRC and connect to SPIFI clock */
    LPC_CGU->IDIV_CTRL[CLK_IDIV_E] = ((1 << 11) | (CLKIN_IRC << 24));
    LPC_CGU->BASE_CLK[CLK_BASE_SPIFI] = ((1 << 11) | (CLKIN_IDIVE << 24));
#endif /* SPIFI_INIT */

    /* Configure PLL1 (MAINPLL) for main clock */
    LPC_CGU->PLL1_CTRL |= 1; /* Power down PLL1 */

    /* Change PLL1 to 108 Mhz (msel=9, 12 MHz*9=108 MHz) */
//    LPC_CGU->PLL1_CTRL = (DIRECT << 7) | (PSEL << 8) | (1 << 11) | ((NSEL-1) << 12)  | ((MSEL-1) << 16) | (CLKIN_PLL1 << 24);
    LPC_CGU->PLL1_CTRL = (1 << 7) | (0 << 8) | (1 << 11) | (0 << 12) | (8 << 16)
                         | (CLKIN_PLL1 << 24);
    while (!(LPC_CGU->PLL1_STAT & 1)); /* Wait for PLL1 to lock */
    WaitUs(100);

    /* Change PLL1 to 204 Mhz (msel=17, 12 MHz*17=204 MHz) */
    LPC_CGU->PLL1_CTRL = (1 << 7) | (0 << 8) | (1 << 11) | (0 << 12) | (16 << 16)
                         | (CLKIN_PLL1 << 24);
    while (!(LPC_CGU->PLL1_STAT & 1)); /* Wait for PLL1 to lock */

    /* Switch main clock to PLL1 */
    LPC_CGU->BASE_CLK[CLK_BASE_MX] = (1 << 11) | (CLKIN_PLL1 << 24);

    /* Switch main clock to PLL1 */
    LPC_CGU->BASE_CLK[CLK_BASE_MX] = (1 << 11) | (CLKIN_PLL1 << 24);

    /* Set USB PLL dividers for 480 MHz */
    LPC_CGU->PLL[CGU_USB_PLL].PLL_MDIV = 0x06167FFA;
    LPC_CGU->PLL[CGU_USB_PLL].PLL_NP_DIV = 0x00302062;
    LPC_CGU->PLL[CGU_USB_PLL].PLL_CTRL = 0x0000081D | (CLKIN_CRYSTAL << 24);

    /* Switch USB0 clock to USB PLL */
    LPC_CGU->BASE_CLK[CLK_BASE_USB0] = (1 << 0) | (1 << 11) | (CLKIN_PLL0USB << 24);

    /* Set IDIVE clock to PLL1/2 = 102 MHz */
    LPC_CGU->IDIV_CTRL[CLK_IDIV_E] = (1 << 2) | (1 << 11) | (CLKIN_PLL1 << 24); // PLL1/2

    /* Set IDIVD clock to ((USBPLL/4) / 2) = 60 MHz and connect to USB1 */
    LPC_CGU->IDIV_CTRL[CLK_IDIV_A] = (3 << 2) | (1 << 11) | (CLKIN_PLL0USB << 24); // USBPLL/4
    LPC_CGU->IDIV_CTRL[CLK_IDIV_D] = (1 << 2) | (1 << 11) | (CLKIN_IDIVA << 24); // IDIVA/2
    LPC_CGU->BASE_CLK[CLK_BASE_USB1] = (1 << 0) | (1 << 11) | (CLKIN_IDIVD << 24);

    /* Configure remaining integer dividers */
    LPC_CGU->IDIV_CTRL[CLK_IDIV_B] = (0 << 2) | (1 << 11) | (CLKIN_IRC << 24); // IRC
    LPC_CGU->IDIV_CTRL[CLK_IDIV_C] = (1 << 2) | (1 << 11) | (CLKIN_PLL1 << 24); // PLL1/2

    LPC_CGU->BASE_CLK[CLK_BASE_UART0] = (1 << 0) | (1 << 11) | (CLKIN_PLL1 << 24);
    LPC_CGU->BASE_CLK[CLK_BASE_UART1] = (1 << 0) | (1 << 11) | (CLKIN_PLL1 << 24);
    LPC_CGU->BASE_CLK[CLK_BASE_UART2] = (1 << 0) | (1 << 11) | (CLKIN_PLL1 << 24);
#endif /* CLOCK_SETUP */
}

/*
 * SystemSetupPins() - Configure MCU pins
 */
void __section(SectionForBootstrapOperations) SystemSetupPins(const PINMUX_GRP_T *mux, uint32_t n)
{
    uint16_t i;

    for (i = 0; i < n; i++) {
        *(mux[i].reg) = mux[i].mode;
    }
}

/*
 * SystemSetupMemory() - Configure external memory
 */
void SystemSetupMemory(void)
{
#if (MEMORY_SETUP)
    /* None required for boards without external memory */
#endif /* MEMORY_SETUP */
}

#if 0 // defined(__FPU_PRESENT) && __FPU_PRESENT == 1
/*
 * fpuInit() - Early initialization of the FPU
 */
void __section(SectionForBootstrapOperations) fpuInit(void)
{
	// from ARM TRM manual:
	//   ; CPACR is located at address 0xE000ED88
	//   LDR.W R0, =0xE000ED88
	//   ; Read CPACR
	//   LDR R1, [R0]
	//   ; Set bits 20-23 to enable CP10 and CP11 coprocessors
	//   ORR R1, R1, #(0xF << 20)
	//   ; Write back the modified value to the CPACR
	//   STR R1, [R0]

	volatile uint32_t *regCpacr = (uint32_t *) LPC_CPACR;
	volatile uint32_t *regMvfr0 = (uint32_t *) SCB_MVFR0;
	volatile uint32_t *regMvfr1 = (uint32_t *) SCB_MVFR1;
	volatile uint32_t Cpacr;
	volatile uint32_t Mvfr0;
	volatile uint32_t Mvfr1;
	char vfpPresent = 0;

	Mvfr0 = *regMvfr0;
	Mvfr1 = *regMvfr1;

	vfpPresent = ((SCB_MVFR0_RESET == Mvfr0) && (SCB_MVFR1_RESET == Mvfr1));

	if (vfpPresent) {
		Cpacr = *regCpacr;
		Cpacr |= (0xF << 20);
		*regCpacr = Cpacr;	// enable CP10 and CP11 for full access
	}

}
#endif /* defined(__FPU_PRESENT) && __FPU_PRESENT == 1 */

/* Approximate delay function */
#define CPU_NANOSEC(x)  (((uint64_t) (x) * SystemCoreClock) / 1000000000)

static void WaitUs(uint32_t us)
{
    uint32_t  cyc = us * CPU_NANOSEC(1000) / 4;
    while (cyc--)
        ;
}

#endif /* !defined(CORE_M0) */

void __section(SectionForBootstrapOperations) BootstrapCode()
{
    SystemInit();

    BootstrapCode_GPIO();

    PrepareImageRegions();
}
