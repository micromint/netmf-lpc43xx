////////////////////////////////////////////////////////////////////////////////
// processor_selector.h for NXP LPC43XX
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

#ifndef _LPC43XX_PROCESSOR_SELECTOR_H_
#define _LPC43XX_PROCESSOR_SELECTOR_H_ 1

#if !defined(PLATFORM_ARM_LPC43XX)
ERROR - WE SHOULD NOT INCLUDE THIS HEADER IF NOT BUILDING A LPC43XX PLATFORM
#endif

#define PLATFORM_ARM_DEFINED

#define LPC43XX

/////////////////////////////////////////////////////////
//
// macros
//

#ifndef GLOBAL_LOCK
#define GLOBAL_LOCK(x)             SmartPtr_IRQ x
#define DISABLE_INTERRUPTS()       SmartPtr_IRQ::ForceDisabled()
#define ENABLE_INTERRUPTS()        SmartPtr_IRQ::ForceEnabled()
#define INTERRUPTS_ENABLED_STATE() SmartPtr_IRQ::GetState()
#endif

#ifndef GLOBAL_LOCK_SOCKETS
#define GLOBAL_LOCK_SOCKETS(x)     SmartPtr_IRQ x
#endif

#ifndef ASSERT_IRQ_MUST_BE_OFF
#if defined(_DEBUG)
#define ASSERT_IRQ_MUST_BE_OFF()   ASSERT(!SmartPtr_IRQ::GetState())
#define ASSERT_IRQ_MUST_BE_ON()    ASSERT( SmartPtr_IRQ::GetState())
#else
#define ASSERT_IRQ_MUST_BE_OFF()
#define ASSERT_IRQ_MUST_BE_ON()
#endif
#endif

#if defined(_DEBUG)
#define USB_DEBUG_STATEMACHINE
#endif
//
// macros
//
/////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////
//
// constants
//

#ifndef TOTAL_USART_PORT
// Port definitions
#define TOTAL_USART_PORT       4
#define COM1                   ConvertCOM_ComHandle(0)
#define COM2                   ConvertCOM_ComHandle(1)
#define COM3                   ConvertCOM_ComHandle(2)
#define COM4                   ConvertCOM_ComHandle(3)

#define TOTAL_USB_CONTROLLER   2
#define USB1                   ConvertCOM_UsbHandle(0)
#define USB2                   ConvertCOM_UsbHandle(1)

#define TOTAL_DEBUG_PORT       1
#define COM_DEBUG              ConvertCOM_DebugHandle(0)

#define COM_MESSAGING          ConvertCOM_MessagingHandle(0)

#define USART_TX_IRQ_INDEX(x)  (24 + x)  // (USART0_IRQn + x)

#define USART_DEFAULT_PORT          COM1
#define USART_DEFAULT_BAUDRATE      115200

#define USB_IRQ_INDEX          8  // USB0_IRQn

#ifndef PLATFORM_DEPENDENT_TX_USART_BUFFER_SIZE
#define PLATFORM_DEPENDENT_TX_USART_BUFFER_SIZE    64  // there is one TX for each usart port
#endif
#ifndef PLATFORM_DEPENDENT_RX_USART_BUFFER_SIZE
#define PLATFORM_DEPENDENT_RX_USART_BUFFER_SIZE    64  // there is one RX for each usart port
#endif

// There is one queue for each pipe of each endpoint
#ifndef PLATFORM_DEPENDENT_USB_QUEUE_PACKET_COUNT
#define PLATFORM_DEPENDENT_USB_QUEUE_PACKET_COUNT  16
#endif
// Single packet size is sizeof(USB_PACKET64) == (USB_MAX_DATA_PACKET_SIZE + 4)
// Max for bulk transfers is 64 for full speed, 512 for high speed. Increasing
// over 128 requires changing USB_CONTROLLER_STATE.MaxPacketSize, etc. from UINT8
// to UINT16. Typically on LPC43xx USB0 is high speed, USB1 is full speed.
#ifndef PLATFORM_DEPENDENT_USB_MAX_DATA_PACKET_SIZE
#define PLATFORM_DEPENDENT_USB_MAX_DATA_PACKET_SIZE  64
#endif

#endif

#define DEFAULT_CLOCK_DIV           1

#if !defined(USART_TX_XOFF_TIMEOUT_INFINITE)
#define USART_TX_XOFF_TIMEOUT_INFINITE   0xFFFFFFFF
#endif

// USART_TX_XOFF_TIMEOUT_TICKS = the number of ticks to leave TX in the XOFF state.  The 
// timeout is a failsafe so that if the XON is lost or an spurious XOFF is received the 
// TX won't be off indefinitely.
//
// The platform selector file should override this value, default to 1min
#if !defined(USART_TX_XOFF_TIMEOUT_TICKS) 
#define USART_TX_XOFF_TIMEOUT_TICKS      (CPU_TicksPerSecond() * 60)
#endif

//
// constants
//
/////////////////////////////////////////////////////////


/////////////////////////////////////////////////////////
//
// macros
//    
#define UNUSED_GPIO_PULLDOWN(x) { (UINT8)MC9328MXL_GPIO::c_Port_##x, (UINT8)RESISTOR_PULLDOWN }
#define UNUSED_GPIO_PULLUP(x)   { (UINT8)MC9328MXL_GPIO::c_Port_##x, (UINT8)RESISTOR_PULLUP   }
#define UNUSED_GPIO_DISABLED(x) { (UINT8)MC9328MXL_GPIO::c_Port_##x, (UINT8)RESISTOR_DISABLED }
//
// macros
//
/////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////
//
// global functions
//

//
// global functions
//
/////////////////////////////////////////////////////////

#include <tinypal.h>
#include <drivers.h>

#include "DeviceCode\LPC43XX.h"
#include <Cores\arm\Include\cpu.h>

#endif
