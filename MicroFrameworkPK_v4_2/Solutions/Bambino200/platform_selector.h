////////////////////////////////////////////////////////////////////////////////
// platform_selector.h
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
// LPC43XX port: Copyright (c) Micromint USA. All rights reserved.
////////////////////////////////////////////////////////////////////////////////

#ifndef _PLATFORM_Bambino200_SELECTOR_H_
#define _PLATFORM_Bambino200_SELECTOR_H_ 1

/////////////////////////////////////////////////////////
//
// processor and features
//

#if defined(PLATFORM_ARM_Bambino200)
#define HAL_SYSTEM_NAME                     "Bambino200"

#define PLATFORM_ARM_LPC43XX
//
// processor and features
//
/////////////////////////////////////////////////////////


/////////////////////////////////////////////////////////
//
// constants
//

#define SYSTEM_CLOCK_HZ                 204000000
#define SYSTEM_CYCLE_CLOCK_HZ           SYSTEM_CLOCK_HZ
#define CLOCK_COMMON_FACTOR             1000000
#define SLOW_CLOCKS_PER_SECOND          12000000
#define SLOW_CLOCKS_TEN_MHZ_GCD         12000000
#define SLOW_CLOCKS_MILLISECOND_GCD     1000

#define SRAM1_MEMORY_Base   0x10000000
#define SRAM1_MEMORY_Size   0x00020000
#define FLASH_MEMORY_Base   0x14000000
#define FLASH_MEMORY_Size   0x00400000

#define TXPROTECTRESISTOR               RESISTOR_DISABLED
#define RXPROTECTRESISTOR               RESISTOR_DISABLED
#define CTSPROTECTRESISTOR              RESISTOR_DISABLED
#define RTSPROTECTRESISTOR              RESISTOR_DISABLED

#define INSTRUMENTATION_H_GPIO_PIN      0

#ifndef DEBUG_SERIAL
  #define DEBUG_TEXT_PORT    USB1
  #define STDIO              USB1
  #define DEBUGGER_PORT      USB1
  #define MESSAGING_PORT     USB1
#else
  #define DEBUG_TEXT_PORT    COM1
  #define STDIO              COM1
  #define DEBUGGER_PORT      COM1
  #define MESSAGING_PORT     COM1
#endif

#define OEM_VERSION_MAJOR 4
#define OEM_VERSION_MINOR 2
#define OEM_VERSION_BUILD 0
#define OEM_VERSION_REVISION 2
#define OEM_INFOSTRING "Copyright (C) Micromint USA LLC"

//
// constants
/////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////
//
// global functions
//

//
// global functions
//
/////////////////////////////////////////////////////////

#include <processor_selector.h>

#endif // PLATFORM_ARM_Bambino200
//
// drivers
/////////////////////////////////////////////////////////

#endif // _PLATFORM_Bambino200_SELECTOR_H_
