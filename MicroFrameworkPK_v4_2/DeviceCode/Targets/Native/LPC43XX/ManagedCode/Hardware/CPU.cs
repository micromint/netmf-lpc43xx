////////////////////////////////////////////////////////////////////////////////
// CPU.cs - CPU enumeration example for NXP LPC43XX
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
using System;
using System.Collections;
using System.Threading;
using System.Runtime.CompilerServices;
using Microsoft.SPOT.Hardware;
using System.IO.Ports;

namespace Microsoft.SPOT.Hardware.LPC43XX
{

    /// <summary>
    /// Specifies identifiers for hardware I/O pins.
    /// </summary>
    public static class Pin
    {
        // On the LPC43xx the CPU pin name and the GPIO pin name are not the same.
        // To simplify runtime logic, encode SCU and GPIO offsets as a pin identifier
        // using constants.
        //   SCU_OFF(group, num) = ((0x80 * group) + (4 * num));
        //   GPIO_OFF(port, pin) = ((port << 5) + pin);
        //   _pin(group, num, port, pin) = ((SCU_OFF(group,num) << 16) + GPIO_OFF(port,pin));
        //   _pin(group, num, port, pin) =  ((((0x80 * group) + (4 * num)) << 16) + ((port << 5) + pin));

        // CPU Pins
        public const Cpu.Pin P0_0 = (Cpu.Pin)((((0x80 * 0x0) + (4 * 0)) << 16) + ((0 << 5) + 0)); // GPIO0[0]
        public const Cpu.Pin P0_1 = (Cpu.Pin)((((0x80 * 0x0) + (4 * 1)) << 16) + ((0 << 5) + 1)); // GPIO0[1]

        public const Cpu.Pin P1_0 = (Cpu.Pin)((((0x80 * 0x1) + (4 * 0)) << 16) + ((0 << 5) + 4)); // GPIO0[4]
        public const Cpu.Pin P1_1 = (Cpu.Pin)((((0x80 * 0x1) + (4 * 1)) << 16) + ((0 << 5) + 8)); // GPIO0[8]
        public const Cpu.Pin P1_2 = (Cpu.Pin)((((0x80 * 0x1) + (4 * 2)) << 16) + ((0 << 5) + 9)); // GPIO0[9]
        public const Cpu.Pin P1_3 = (Cpu.Pin)((((0x80 * 0x1) + (4 * 3)) << 16) + ((0 << 5) + 10)); // GPIO0[10]
        public const Cpu.Pin P1_4 = (Cpu.Pin)((((0x80 * 0x1) + (4 * 4)) << 16) + ((0 << 5) + 11)); // GPIO0[11]
        public const Cpu.Pin P1_5 = (Cpu.Pin)((((0x80 * 0x1) + (4 * 5)) << 16) + ((1 << 5) + 8)); // GPIO1[8]
        public const Cpu.Pin P1_6 = (Cpu.Pin)((((0x80 * 0x1) + (4 * 6)) << 16) + ((1 << 5) + 9)); // GPIO1[9]
        public const Cpu.Pin P1_7 = (Cpu.Pin)((((0x80 * 0x1) + (4 * 7)) << 16) + ((1 << 5) + 0)); // GPIO1[0]
        public const Cpu.Pin P1_8 = (Cpu.Pin)((((0x80 * 0x1) + (4 * 8)) << 16) + ((1 << 5) + 1)); // GPIO1[1]
        public const Cpu.Pin P1_9 = (Cpu.Pin)((((0x80 * 0x1) + (4 * 9)) << 16) + ((1 << 5) + 2)); // GPIO1[2]
        public const Cpu.Pin P1_10 = (Cpu.Pin)((((0x80 * 0x1) + (4 * 10)) << 16) + ((1 << 5) + 3)); // GPIO1[3]
        public const Cpu.Pin P1_11 = (Cpu.Pin)((((0x80 * 0x1) + (4 * 11)) << 16) + ((1 << 5) + 4)); // GPIO1[4]
        public const Cpu.Pin P1_12 = (Cpu.Pin)((((0x80 * 0x1) + (4 * 12)) << 16) + ((1 << 5) + 5)); // GPIO1[5]
        public const Cpu.Pin P1_13 = (Cpu.Pin)((((0x80 * 0x1) + (4 * 13)) << 16) + ((1 << 5) + 6)); // GPIO1[6]
        public const Cpu.Pin P1_14 = (Cpu.Pin)((((0x80 * 0x1) + (4 * 14)) << 16) + ((1 << 5) + 7)); // GPIO1[7]
        public const Cpu.Pin P1_15 = (Cpu.Pin)((((0x80 * 0x1) + (4 * 15)) << 16) + ((0 << 5) + 2)); // GPIO0[2]
        public const Cpu.Pin P1_16 = (Cpu.Pin)((((0x80 * 0x1) + (4 * 16)) << 16) + ((0 << 5) + 3)); // GPIO0[3]
        public const Cpu.Pin P1_17 = (Cpu.Pin)((((0x80 * 0x1) + (4 * 17)) << 16) + ((0 << 5) + 12)); // GPIO0[12] high-drive
        public const Cpu.Pin P1_18 = (Cpu.Pin)((((0x80 * 0x1) + (4 * 18)) << 16) + ((0 << 5) + 13)); // GPIO0[13]
        public const Cpu.Pin P1_19 = (Cpu.Pin)((((0x80 * 0x1) + (4 * 19)) << 16) + ((15 << 5) + 0));
        public const Cpu.Pin P1_20 = (Cpu.Pin)((((0x80 * 0x1) + (4 * 20)) << 16) + ((0 << 5) + 15)); // GPIO0[15]

        public const Cpu.Pin P2_0 = (Cpu.Pin)((((0x80 * 0x2) + (4 * 0)) << 16) + ((5 << 5) + 0)); // GPIO5[0]
        public const Cpu.Pin P2_1 = (Cpu.Pin)((((0x80 * 0x2) + (4 * 1)) << 16) + ((5 << 5) + 1)); // GPIO5[1]
        public const Cpu.Pin P2_2 = (Cpu.Pin)((((0x80 * 0x2) + (4 * 2)) << 16) + ((5 << 5) + 2)); // GPIO5[2]
        public const Cpu.Pin P2_3 = (Cpu.Pin)((((0x80 * 0x2) + (4 * 3)) << 16) + ((5 << 5) + 3)); // GPIO5[3]  high-drive
        public const Cpu.Pin P2_4 = (Cpu.Pin)((((0x80 * 0x2) + (4 * 4)) << 16) + ((5 << 5) + 4)); // GPIO5[4]  high-drive
        public const Cpu.Pin P2_5 = (Cpu.Pin)((((0x80 * 0x2) + (4 * 5)) << 16) + ((5 << 5) + 5)); // GPIO5[5]  high-drive
        public const Cpu.Pin P2_6 = (Cpu.Pin)((((0x80 * 0x2) + (4 * 6)) << 16) + ((5 << 5) + 6)); // GPIO5[6]
        public const Cpu.Pin P2_7 = (Cpu.Pin)((((0x80 * 0x2) + (4 * 7)) << 16) + ((0 << 5) + 7)); // GPIO0[7]
        public const Cpu.Pin P2_8 = (Cpu.Pin)((((0x80 * 0x2) + (4 * 8)) << 16) + ((5 << 5) + 7)); // GPIO5[7]
        public const Cpu.Pin P2_9 = (Cpu.Pin)((((0x80 * 0x2) + (4 * 9)) << 16) + ((1 << 5) + 10)); // GPIO1[10]
        public const Cpu.Pin P2_10 = (Cpu.Pin)((((0x80 * 0x2) + (4 * 10)) << 16) + ((0 << 5) + 14)); // GPIO0[14]
        public const Cpu.Pin P2_11 = (Cpu.Pin)((((0x80 * 0x2) + (4 * 11)) << 16) + ((1 << 5) + 11)); // GPIO1[11]
        public const Cpu.Pin P2_12 = (Cpu.Pin)((((0x80 * 0x2) + (4 * 12)) << 16) + ((1 << 5) + 12)); // GPIO1[12]
        public const Cpu.Pin P2_13 = (Cpu.Pin)((((0x80 * 0x2) + (4 * 13)) << 16) + ((1 << 5) + 13)); // GPIO1[13]

        public const Cpu.Pin P3_0 = (Cpu.Pin)((((0x80 * 0x3) + (4 * 0)) << 16) + ((15 << 5) + 0));
        public const Cpu.Pin P3_1 = (Cpu.Pin)((((0x80 * 0x3) + (4 * 1)) << 16) + ((5 << 5) + 8)); // GPIO5[8]
        public const Cpu.Pin P3_2 = (Cpu.Pin)((((0x80 * 0x3) + (4 * 2)) << 16) + ((5 << 5) + 9)); // GPIO5[9]
        public const Cpu.Pin P3_3 = (Cpu.Pin)((((0x80 * 0x3) + (4 * 3)) << 16) + ((15 << 5) + 0));
        public const Cpu.Pin P3_4 = (Cpu.Pin)((((0x80 * 0x3) + (4 * 4)) << 16) + ((1 << 5) + 14)); // GPIO1[14]
        public const Cpu.Pin P3_5 = (Cpu.Pin)((((0x80 * 0x3) + (4 * 5)) << 16) + ((1 << 5) + 15)); // GPIO1[15]
        public const Cpu.Pin P3_6 = (Cpu.Pin)((((0x80 * 0x3) + (4 * 6)) << 16) + ((0 << 5) + 6)); // GPIO0[6]
        public const Cpu.Pin P3_7 = (Cpu.Pin)((((0x80 * 0x3) + (4 * 7)) << 16) + ((5 << 5) + 10)); // GPIO5[10]
        public const Cpu.Pin P3_8 = (Cpu.Pin)((((0x80 * 0x3) + (4 * 8)) << 16) + ((5 << 5) + 11)); // GPIO5[11]

        public const Cpu.Pin P4_0 = (Cpu.Pin)((((0x80 * 0x4) + (4 * 0)) << 16) + ((2 << 5) + 0)); // GPIO2[0]
        public const Cpu.Pin P4_1 = (Cpu.Pin)((((0x80 * 0x4) + (4 * 1)) << 16) + ((2 << 5) + 1)); // GPIO2[1]
        public const Cpu.Pin P4_2 = (Cpu.Pin)((((0x80 * 0x4) + (4 * 2)) << 16) + ((2 << 5) + 2)); // GPIO2[2]
        public const Cpu.Pin P4_3 = (Cpu.Pin)((((0x80 * 0x4) + (4 * 3)) << 16) + ((2 << 5) + 3)); // GPIO2[3]
        public const Cpu.Pin P4_4 = (Cpu.Pin)((((0x80 * 0x4) + (4 * 4)) << 16) + ((2 << 5) + 4)); // GPIO2[4]
        public const Cpu.Pin P4_5 = (Cpu.Pin)((((0x80 * 0x4) + (4 * 5)) << 16) + ((2 << 5) + 5)); // GPIO2[5]
        public const Cpu.Pin P4_6 = (Cpu.Pin)((((0x80 * 0x4) + (4 * 6)) << 16) + ((2 << 5) + 6)); // GPIO2[6]
        public const Cpu.Pin P4_7 = (Cpu.Pin)((((0x80 * 0x4) + (4 * 7)) << 16) + ((15 << 5) + 0));
        public const Cpu.Pin P4_8 = (Cpu.Pin)((((0x80 * 0x4) + (4 * 8)) << 16) + ((5 << 5) + 12)); // GPIO5[12]
        public const Cpu.Pin P4_9 = (Cpu.Pin)((((0x80 * 0x4) + (4 * 9)) << 16) + ((5 << 5) + 13)); // GPIO5[13]
        public const Cpu.Pin P4_10 = (Cpu.Pin)((((0x80 * 0x4) + (4 * 10)) << 16) + ((5 << 5) + 14)); // GPIO5[14]

        public const Cpu.Pin P5_0 = (Cpu.Pin)((((0x80 * 0x5) + (4 * 0)) << 16) + ((2 << 5) + 9)); // GPIO2[9]
        public const Cpu.Pin P5_1 = (Cpu.Pin)((((0x80 * 0x5) + (4 * 1)) << 16) + ((2 << 5) + 10)); // GPIO2[10]
        public const Cpu.Pin P5_2 = (Cpu.Pin)((((0x80 * 0x5) + (4 * 2)) << 16) + ((2 << 5) + 11)); // GPIO2[11]
        public const Cpu.Pin P5_3 = (Cpu.Pin)((((0x80 * 0x5) + (4 * 3)) << 16) + ((2 << 5) + 12)); // GPIO2[12]
        public const Cpu.Pin P5_4 = (Cpu.Pin)((((0x80 * 0x5) + (4 * 4)) << 16) + ((2 << 5) + 13)); // GPIO2[13]
        public const Cpu.Pin P5_5 = (Cpu.Pin)((((0x80 * 0x5) + (4 * 5)) << 16) + ((2 << 5) + 14)); // GPIO2[14]
        public const Cpu.Pin P5_6 = (Cpu.Pin)((((0x80 * 0x5) + (4 * 6)) << 16) + ((2 << 5) + 15)); // GPIO2[15]
        public const Cpu.Pin P5_7 = (Cpu.Pin)((((0x80 * 0x5) + (4 * 7)) << 16) + ((2)) << 16) + ((7)); // GPIO2[7]

        public const Cpu.Pin P6_0 = (Cpu.Pin)((((0x80 * 0x6) + (4 * 0)) << 16) + ((15 << 5) + 0));
        public const Cpu.Pin P6_1 = (Cpu.Pin)((((0x80 * 0x6) + (4 * 1)) << 16) + ((3 << 5) + 0)); // GPIO3[0]
        public const Cpu.Pin P6_2 = (Cpu.Pin)((((0x80 * 0x6) + (4 * 2)) << 16) + ((3 << 5) + 1)); // GPIO3[1]
        public const Cpu.Pin P6_3 = (Cpu.Pin)((((0x80 * 0x6) + (4 * 3)) << 16) + ((3 << 5) + 2)); // GPIO3[2]
        public const Cpu.Pin P6_4 = (Cpu.Pin)((((0x80 * 0x6) + (4 * 4)) << 16) + ((3 << 5) + 3)); // GPIO3[3]
        public const Cpu.Pin P6_5 = (Cpu.Pin)((((0x80 * 0x6) + (4 * 5)) << 16) + ((3 << 5) + 4)); // GPIO3[4]
        public const Cpu.Pin P6_6 = (Cpu.Pin)((((0x80 * 0x6) + (4 * 6)) << 16) + ((0 << 5) + 5)); // GPIO0[5]
        public const Cpu.Pin P6_7 = (Cpu.Pin)((((0x80 * 0x6) + (4 * 7)) << 16) + ((5 << 5) + 15)); // GPIO5[15]
        public const Cpu.Pin P6_8 = (Cpu.Pin)((((0x80 * 0x6) + (4 * 8)) << 16) + ((5 << 5) + 16)); // GPIO5[16]
        public const Cpu.Pin P6_9 = (Cpu.Pin)((((0x80 * 0x6) + (4 * 9)) << 16) + ((3 << 5) + 5)); // GPIO3[5]
        public const Cpu.Pin P6_10 = (Cpu.Pin)((((0x80 * 0x6) + (4 * 10)) << 16) + ((3 << 5) + 6)); // GPIO3[6]
        public const Cpu.Pin P6_11 = (Cpu.Pin)((((0x80 * 0x6) + (4 * 11)) << 16) + ((3 << 5) + 7)); // GPIO3[7]
        public const Cpu.Pin P6_12 = (Cpu.Pin)((((0x80 * 0x6) + (4 * 12)) << 16) + ((2 << 5) + 8)); // GPIO2[8]

        public const Cpu.Pin P7_0 = (Cpu.Pin)((((0x80 * 0x7) + (4 * 0)) << 16) + ((3 << 5) + 8)); // GPIO3[8]
        public const Cpu.Pin P7_1 = (Cpu.Pin)((((0x80 * 0x7) + (4 * 1)) << 16) + ((3 << 5) + 9)); // GPIO3[9]
        public const Cpu.Pin P7_2 = (Cpu.Pin)((((0x80 * 0x7) + (4 * 2)) << 16) + ((3 << 5) + 10)); // GPIO3[10]
        public const Cpu.Pin P7_3 = (Cpu.Pin)((((0x80 * 0x7) + (4 * 3)) << 16) + ((3 << 5) + 11)); // GPIO3[11]
        public const Cpu.Pin P7_4 = (Cpu.Pin)((((0x80 * 0x7) + (4 * 4)) << 16) + ((3 << 5) + 12)); // GPIO3[12]
        public const Cpu.Pin P7_5 = (Cpu.Pin)((((0x80 * 0x7) + (4 * 5)) << 16) + ((3 << 5) + 13)); // GPIO3[13]
        public const Cpu.Pin P7_6 = (Cpu.Pin)((((0x80 * 0x7) + (4 * 6)) << 16) + ((3 << 5) + 14)); // GPIO3[14]
        public const Cpu.Pin P7_7 = (Cpu.Pin)((((0x80 * 0x7) + (4 * 7)) << 16) + ((3 << 5) + 15)); // GPIO3[15]

        public const Cpu.Pin P8_0 = (Cpu.Pin)((((0x80 * 0x8) + (4 * 8)) << 16) + ((4 << 5) + 0)); // GPIO4[0]  high-drive
        public const Cpu.Pin P8_1 = (Cpu.Pin)((((0x80 * 0x9) + (4 * 0)) << 16) + ((4 << 5) + 1)); // GPIO4[1]  high-drive
        public const Cpu.Pin P8_2 = (Cpu.Pin)((((0x80 * 0x9) + (4 * 1)) << 16) + ((4 << 5) + 2)); // GPIO4[2]  high-drive
        public const Cpu.Pin P8_3 = (Cpu.Pin)((((0x80 * 0x9) + (4 * 2)) << 16) + ((4 << 5) + 3)); // GPIO4[3]
        public const Cpu.Pin P8_4 = (Cpu.Pin)((((0x80 * 0x8) + (4 * 4)) << 16) + ((4 << 5) + 4)); // GPIO4[4]
        public const Cpu.Pin P8_5 = (Cpu.Pin)((((0x80 * 0x8) + (4 * 5)) << 16) + ((4 << 5) + 5)); // GPIO4[5]
        public const Cpu.Pin P8_6 = (Cpu.Pin)((((0x80 * 0x8) + (4 * 6)) << 16) + ((4 << 5) + 6)); // GPIO4[6]
        public const Cpu.Pin P8_7 = (Cpu.Pin)((((0x80 * 0x8) + (4 * 7)) << 16) + ((4 << 5) + 7)); // GPIO4[7]
        public const Cpu.Pin P8_8 = (Cpu.Pin)((((0x80 * 0x8) + (4 * 8)) << 16) + ((15 << 5) + 0));

        public const Cpu.Pin P9_0 = (Cpu.Pin)((((0x80 * 0x9) + (4 * 0)) << 16) + ((4 << 5) + 12)); // GPIO4[12]
        public const Cpu.Pin P9_1 = (Cpu.Pin)((((0x80 * 0x9) + (4 * 1)) << 16) + ((4 << 5) + 13)); // GPIO4[13]
        public const Cpu.Pin P9_2 = (Cpu.Pin)((((0x80 * 0x9) + (4 * 2)) << 16) + ((4 << 5) + 14)); // GPIO4[14]
        public const Cpu.Pin P9_3 = (Cpu.Pin)((((0x80 * 0x9) + (4 * 3)) << 16) + ((4 << 5) + 15)); // GPIO4[15]
        public const Cpu.Pin P9_4 = (Cpu.Pin)((((0x80 * 0x9) + (4 * 4)) << 16) + ((5 << 5) + 17)); // GPIO5[17]
        public const Cpu.Pin P9_5 = (Cpu.Pin)((((0x80 * 0x9) + (4 * 5)) << 16) + ((5 << 5) + 18)); // GPIO5[18]
        public const Cpu.Pin P9_6 = (Cpu.Pin)((((0x80 * 0x9) + (4 * 6)) << 16) + ((4 << 5) + 11)); // GPIO4[11]

        public const Cpu.Pin PA_0 = (Cpu.Pin)((((0x80 * 0xA) + (4 * 0)) << 16) + ((15 << 5) + 0));
        public const Cpu.Pin PA_1 = (Cpu.Pin)((((0x80 * 0xA) + (4 * 1)) << 16) + ((4 << 5) + 8)); // GPIO4[8]  high-drive
        public const Cpu.Pin PA_2 = (Cpu.Pin)((((0x80 * 0xA) + (4 * 2)) << 16) + ((4 << 5) + 9)); // GPIO4[9]  high-drive
        public const Cpu.Pin PA_3 = (Cpu.Pin)((((0x80 * 0xA) + (4 * 3)) << 16) + ((4 << 5) + 10)); // GPIO4[10] high-drive
        public const Cpu.Pin PA_4 = (Cpu.Pin)((((0x80 * 0xA) + (4 * 4)) << 16) + ((5 << 5) + 19)); // GPIO5[19]

        public const Cpu.Pin PB_0 = (Cpu.Pin)((((0x80 * 0xB) + (4 * 0)) << 16) + ((5 << 5) + 20)); // GPIO5[20]
        public const Cpu.Pin PB_1 = (Cpu.Pin)((((0x80 * 0xB) + (4 * 1)) << 16) + ((5 << 5) + 21)); // GPIO5[21]
        public const Cpu.Pin PB_2 = (Cpu.Pin)((((0x80 * 0xB) + (4 * 2)) << 16) + ((5 << 5) + 22)); // GPIO5[22]
        public const Cpu.Pin PB_3 = (Cpu.Pin)((((0x80 * 0xB) + (4 * 3)) << 16) + ((5 << 5) + 23)); // GPIO5[23]
        public const Cpu.Pin PB_4 = (Cpu.Pin)((((0x80 * 0xB) + (4 * 4)) << 16) + ((5 << 5) + 24)); // GPIO5[24]
        public const Cpu.Pin PB_5 = (Cpu.Pin)((((0x80 * 0xB) + (4 * 5)) << 16) + ((5 << 5) + 25)); // GPIO5[25]
        public const Cpu.Pin PB_6 = (Cpu.Pin)((((0x80 * 0xB) + (4 * 6)) << 16) + ((5 << 5) + 26)); // GPIO5[26]

        public const Cpu.Pin PC_0 = (Cpu.Pin)((((0x80 * 0xC) + (4 * 0)) << 16) + ((15 << 5) + 0));
        public const Cpu.Pin PC_1 = (Cpu.Pin)((((0x80 * 0xC) + (4 * 1)) << 16) + ((6 << 5) + 0)); // GPIO6[0]
        public const Cpu.Pin PC_2 = (Cpu.Pin)((((0x80 * 0xC) + (4 * 2)) << 16) + ((6 << 5) + 1)); // GPIO6[1]
        public const Cpu.Pin PC_3 = (Cpu.Pin)((((0x80 * 0xC) + (4 * 3)) << 16) + ((6 << 5) + 2)); // GPIO6[2]
        public const Cpu.Pin PC_4 = (Cpu.Pin)((((0x80 * 0xC) + (4 * 4)) << 16) + ((6 << 5) + 3)); // GPIO6[3]
        public const Cpu.Pin PC_5 = (Cpu.Pin)((((0x80 * 0xC) + (4 * 5)) << 16) + ((6 << 5) + 4)); // GPIO6[4]
        public const Cpu.Pin PC_6 = (Cpu.Pin)((((0x80 * 0xC) + (4 * 6)) << 16) + ((6 << 5) + 5)); // GPIO6[5]
        public const Cpu.Pin PC_7 = (Cpu.Pin)((((0x80 * 0xC) + (4 * 7)) << 16) + ((6 << 5) + 6)); // GPIO6[6]
        public const Cpu.Pin PC_8 = (Cpu.Pin)((((0x80 * 0xC) + (4 * 8)) << 16) + ((6 << 5) + 7)); // GPIO6[7]
        public const Cpu.Pin PC_9 = (Cpu.Pin)((((0x80 * 0xC) + (4 * 9)) << 16) + ((6 << 5) + 8)); // GPIO6[8]
        public const Cpu.Pin PC_10 = (Cpu.Pin)((((0x80 * 0xC) + (4 * 10)) << 16) + ((6 << 5) + 9)); // GPIO6[9]
        public const Cpu.Pin PC_11 = (Cpu.Pin)((((0x80 * 0xC) + (4 * 11)) << 16) + ((6 << 5) + 10)); // GPIO6[10]
        public const Cpu.Pin PC_12 = (Cpu.Pin)((((0x80 * 0xC) + (4 * 12)) << 16) + ((6 << 5) + 11)); // GPIO6[11]
        public const Cpu.Pin PC_13 = (Cpu.Pin)((((0x80 * 0xC) + (4 * 13)) << 16) + ((6 << 5) + 12)); // GPIO6[12]
        public const Cpu.Pin PC_14 = (Cpu.Pin)((((0x80 * 0xC) + (4 * 14)) << 16) + ((6 << 5) + 13)); // GPIO6[13]

        public const Cpu.Pin PD_0 = (Cpu.Pin)((((0x80 * 0xD) + (4 * 0)) << 16) + ((6 << 5) + 14)); // GPIO6[14]
        public const Cpu.Pin PD_1 = (Cpu.Pin)((((0x80 * 0xD) + (4 * 1)) << 16) + ((6 << 5) + 15)); // GPIO6[15]
        public const Cpu.Pin PD_2 = (Cpu.Pin)((((0x80 * 0xD) + (4 * 2)) << 16) + ((6 << 5) + 16)); // GPIO6[16]
        public const Cpu.Pin PD_3 = (Cpu.Pin)((((0x80 * 0xD) + (4 * 3)) << 16) + ((6 << 5) + 17)); // GPIO6[17]
        public const Cpu.Pin PD_4 = (Cpu.Pin)((((0x80 * 0xD) + (4 * 4)) << 16) + ((6 << 5) + 18)); // GPIO6[18]
        public const Cpu.Pin PD_5 = (Cpu.Pin)((((0x80 * 0xD) + (4 * 5)) << 16) + ((6 << 5) + 19)); // GPIO6[19]
        public const Cpu.Pin PD_6 = (Cpu.Pin)((((0x80 * 0xD) + (4 * 6)) << 16) + ((6 << 5) + 20)); // GPIO6[20]
        public const Cpu.Pin PD_7 = (Cpu.Pin)((((0x80 * 0xD) + (4 * 7)) << 16) + ((6 << 5) + 21)); // GPIO6[21]
        public const Cpu.Pin PD_8 = (Cpu.Pin)((((0x80 * 0xD) + (4 * 8)) << 16) + ((6 << 5) + 22)); // GPIO6[22]
        public const Cpu.Pin PD_9 = (Cpu.Pin)((((0x80 * 0xD) + (4 * 9)) << 16) + ((6 << 5) + 23)); // GPIO6[23]
        public const Cpu.Pin PD_10 = (Cpu.Pin)((((0x80 * 0xD) + (4 * 10)) << 16) + ((6 << 5) + 24)); // GPIO6[24]
        public const Cpu.Pin PD_11 = (Cpu.Pin)((((0x80 * 0xD) + (4 * 11)) << 16) + ((6 << 5) + 25)); // GPIO6[25]
        public const Cpu.Pin PD_12 = (Cpu.Pin)((((0x80 * 0xD) + (4 * 12)) << 16) + ((6 << 5) + 26)); // GPIO6[26]
        public const Cpu.Pin PD_13 = (Cpu.Pin)((((0x80 * 0xD) + (4 * 13)) << 16) + ((6 << 5) + 27)); // GPIO6[27]
        public const Cpu.Pin PD_14 = (Cpu.Pin)((((0x80 * 0xD) + (4 * 14)) << 16) + ((6 << 5) + 28)); // GPIO6[28]
        public const Cpu.Pin PD_15 = (Cpu.Pin)((((0x80 * 0xD) + (4 * 15)) << 16) + ((6 << 5) + 29)); // GPIO6[29]
        public const Cpu.Pin PD_16 = (Cpu.Pin)((((0x80 * 0xD) + (4 * 16)) << 16) + ((6 << 5) + 30)); // GPIO6[30]

        public const Cpu.Pin PE_0 = (Cpu.Pin)((((0x80 * 0xE) + (4 * 0)) << 16) + ((7 << 5) + 0)); // GPIO7[0]
        public const Cpu.Pin PE_1 = (Cpu.Pin)((((0x80 * 0xE) + (4 * 1)) << 16) + ((7 << 5) + 1)); // GPIO7[1]
        public const Cpu.Pin PE_2 = (Cpu.Pin)((((0x80 * 0xE) + (4 * 2)) << 16) + ((7 << 5) + 2)); // GPIO7[2]
        public const Cpu.Pin PE_3 = (Cpu.Pin)((((0x80 * 0xE) + (4 * 3)) << 16) + ((7 << 5) + 3)); // GPIO7[3]
        public const Cpu.Pin PE_4 = (Cpu.Pin)((((0x80 * 0xE) + (4 * 4)) << 16) + ((7 << 5) + 4)); // GPIO7[4]
        public const Cpu.Pin PE_5 = (Cpu.Pin)((((0x80 * 0xE) + (4 * 5)) << 16) + ((7 << 5) + 5)); // GPIO7[5]
        public const Cpu.Pin PE_6 = (Cpu.Pin)((((0x80 * 0xE) + (4 * 6)) << 16) + ((7 << 5) + 6)); // GPIO7[6]
        public const Cpu.Pin PE_7 = (Cpu.Pin)((((0x80 * 0xE) + (4 * 7)) << 16) + ((7 << 5) + 7)); // GPIO7[7]
        public const Cpu.Pin PE_8 = (Cpu.Pin)((((0x80 * 0xE) + (4 * 8)) << 16) + ((7 << 5) + 8)); // GPIO7[8]
        public const Cpu.Pin PE_9 = (Cpu.Pin)((((0x80 * 0xE) + (4 * 9)) << 16) + ((7 << 5) + 9)); // GPIO7[9]
        public const Cpu.Pin PE_10 = (Cpu.Pin)((((0x80 * 0xE) + (4 * 10)) << 16) + ((7 << 5) + 10)); // GPIO7[10]
        public const Cpu.Pin PE_11 = (Cpu.Pin)((((0x80 * 0xE) + (4 * 11)) << 16) + ((7 << 5) + 11)); // GPIO7[11]
        public const Cpu.Pin PE_12 = (Cpu.Pin)((((0x80 * 0xE) + (4 * 12)) << 16) + ((7 << 5) + 12)); // GPIO7[12]
        public const Cpu.Pin PE_13 = (Cpu.Pin)((((0x80 * 0xE) + (4 * 13)) << 16) + ((7 << 5) + 13)); // GPIO7[13]
        public const Cpu.Pin PE_14 = (Cpu.Pin)((((0x80 * 0xE) + (4 * 14)) << 16) + ((7 << 5) + 14)); // GPIO7[14]
        public const Cpu.Pin PE_15 = (Cpu.Pin)((((0x80 * 0xE) + (4 * 15)) << 16) + ((7 << 5) + 15)); // GPIO7[15]

        public const Cpu.Pin PF_0 = (Cpu.Pin)((((0x80 * 0xF) + (4 * 0)) << 16) + ((15 << 5) + 0));
        public const Cpu.Pin PF_1 = (Cpu.Pin)((((0x80 * 0xF) + (4 * 1)) << 16) + ((7 << 5) + 16)); // GPIO7[16]
        public const Cpu.Pin PF_2 = (Cpu.Pin)((((0x80 * 0xF) + (4 * 2)) << 16) + ((7 << 5) + 17)); // GPIO7[17]
        public const Cpu.Pin PF_3 = (Cpu.Pin)((((0x80 * 0xF) + (4 * 3)) << 16) + ((7 << 5) + 18)); // GPIO7[18]
        public const Cpu.Pin PF_4 = (Cpu.Pin)((((0x80 * 0xF) + (4 * 4)) << 16) + ((15 << 5) + 0));
        public const Cpu.Pin PF_5 = (Cpu.Pin)((((0x80 * 0xF) + (4 * 5)) << 16) + ((7 << 5) + 19)); // GPIO7[19]
        public const Cpu.Pin PF_6 = (Cpu.Pin)((((0x80 * 0xF) + (4 * 6)) << 16) + ((7 << 5) + 20)); // GPIO7[20]
        public const Cpu.Pin PF_7 = (Cpu.Pin)((((0x80 * 0xF) + (4 * 7)) << 16) + ((7 << 5) + 21)); // GPIO7[21]
        public const Cpu.Pin PF_8 = (Cpu.Pin)((((0x80 * 0xF) + (4 * 8)) << 16) + ((7 << 5) + 22)); // GPIO7[22]
        public const Cpu.Pin PF_9 = (Cpu.Pin)((((0x80 * 0xF) + (4 * 9)) << 16) + ((7 << 5) + 23)); // GPIO7[23]
        public const Cpu.Pin PF_10 = (Cpu.Pin)((((0x80 * 0xF) + (4 * 10)) << 16) + ((7 << 5) + 24)); // GPIO7[24]
        public const Cpu.Pin PF_11 = (Cpu.Pin)((((0x80 * 0xF) + (4 * 11)) << 16) + ((7 << 5) + 25)); // GPIO7[25]

        // GPIO Pins
        public const Cpu.Pin GPIO0_0 = P0_0;
        public const Cpu.Pin GPIO0_1 = P0_1 ;
        public const Cpu.Pin GPIO0_2 = P1_15;
        public const Cpu.Pin GPIO0_3 = P1_16;
        public const Cpu.Pin GPIO0_4 = P1_0;
        public const Cpu.Pin GPIO0_5 = P6_6;
        public const Cpu.Pin GPIO0_6 = P3_6;
        public const Cpu.Pin GPIO0_7 = P2_7;
        public const Cpu.Pin GPIO0_8 = P1_1;
        public const Cpu.Pin GPIO0_9 = P1_2;
        public const Cpu.Pin GPIO0_10 = P1_3;
        public const Cpu.Pin GPIO0_11 = P1_4;
        public const Cpu.Pin GPIO0_12 = P1_17;
        public const Cpu.Pin GPIO0_13 = P1_18;
        public const Cpu.Pin GPIO0_14 = P2_10;
        public const Cpu.Pin GPIO0_15 = P1_20;

        public const Cpu.Pin GPIO1_0 = P1_7;
        public const Cpu.Pin GPIO1_1 = P1_8;
        public const Cpu.Pin GPIO1_2 = P1_9;
        public const Cpu.Pin GPIO1_3 = P1_10;
        public const Cpu.Pin GPIO1_4 = P1_11;
        public const Cpu.Pin GPIO1_5 = P1_12;
        public const Cpu.Pin GPIO1_6 = P1_13;
        public const Cpu.Pin GPIO1_7 = P1_14;
        public const Cpu.Pin GPIO1_8 = P1_5;
        public const Cpu.Pin GPIO1_9 = P1_6;
        public const Cpu.Pin GPIO1_10 = P2_9;
        public const Cpu.Pin GPIO1_11 = P2_11;
        public const Cpu.Pin GPIO1_12 = P2_12;
        public const Cpu.Pin GPIO1_13 = P2_13;
        public const Cpu.Pin GPIO1_14 = P3_4;
        public const Cpu.Pin GPIO1_15 = P3_5;

        public const Cpu.Pin GPIO2_0 = P4_0;
        public const Cpu.Pin GPIO2_1 = P4_1;
        public const Cpu.Pin GPIO2_2 = P4_2;
        public const Cpu.Pin GPIO2_3 = P4_3;
        public const Cpu.Pin GPIO2_4 = P4_4;
        public const Cpu.Pin GPIO2_5 = P4_5;
        public const Cpu.Pin GPIO2_6 = P4_6;
        public const Cpu.Pin GPIO2_7 = P5_7;
        public const Cpu.Pin GPIO2_8 = P6_12;
        public const Cpu.Pin GPIO2_9 = P5_0;
        public const Cpu.Pin GPIO2_10 = P5_1;
        public const Cpu.Pin GPIO2_11 = P5_2;
        public const Cpu.Pin GPIO2_12 = P5_3;
        public const Cpu.Pin GPIO2_13 = P5_4;
        public const Cpu.Pin GPIO2_14 = P5_5;
        public const Cpu.Pin GPIO2_15 = P5_6;

        public const Cpu.Pin GPIO3_0 = P6_1;
        public const Cpu.Pin GPIO3_1 = P6_2;
        public const Cpu.Pin GPIO3_2 = P6_3;
        public const Cpu.Pin GPIO3_3 = P6_4;
        public const Cpu.Pin GPIO3_4 = P6_5;
        public const Cpu.Pin GPIO3_5 = P6_9;
        public const Cpu.Pin GPIO3_6 = P6_10;
        public const Cpu.Pin GPIO3_7 = P6_11;
        public const Cpu.Pin GPIO3_8 = P7_0;
        public const Cpu.Pin GPIO3_9 = P7_1;
        public const Cpu.Pin GPIO3_10 = P7_2;
        public const Cpu.Pin GPIO3_11 = P7_3;
        public const Cpu.Pin GPIO3_12 = P7_4;
        public const Cpu.Pin GPIO3_13 = P7_5;
        public const Cpu.Pin GPIO3_14 = P7_6;
        public const Cpu.Pin GPIO3_15 = P7_7;

        public const Cpu.Pin GPIO4_0 = P8_0;
        public const Cpu.Pin GPIO4_1 = P8_1;
        public const Cpu.Pin GPIO4_2 = P8_2;
        public const Cpu.Pin GPIO4_3 = P8_3;
        public const Cpu.Pin GPIO4_4 = P8_4;
        public const Cpu.Pin GPIO4_5 = P8_5;
        public const Cpu.Pin GPIO4_6 = P8_6;
        public const Cpu.Pin GPIO4_7 = P8_7;
        public const Cpu.Pin GPIO4_8 = PA_1;
        public const Cpu.Pin GPIO4_9 = PA_2;
        public const Cpu.Pin GPIO4_10 = PA_3;
        public const Cpu.Pin GPIO4_11 = P9_6;
        public const Cpu.Pin GPIO4_12 = P9_0;
        public const Cpu.Pin GPIO4_13 = P9_1;
        public const Cpu.Pin GPIO4_14 = P9_2;
        public const Cpu.Pin GPIO4_15 = P9_3;

        public const Cpu.Pin GPIO5_0 = P2_0;
        public const Cpu.Pin GPIO5_1 = P2_1;
        public const Cpu.Pin GPIO5_2 = P2_2;
        public const Cpu.Pin GPIO5_3 = P2_3;
        public const Cpu.Pin GPIO5_4 = P2_4;
        public const Cpu.Pin GPIO5_5 = P2_5;
        public const Cpu.Pin GPIO5_6 = P2_6;
        public const Cpu.Pin GPIO5_7 = P2_8;
        public const Cpu.Pin GPIO5_8 = P3_1;
        public const Cpu.Pin GPIO5_9 = P3_2;
        public const Cpu.Pin GPIO5_10 = P3_7;
        public const Cpu.Pin GPIO5_11 = P3_8;
        public const Cpu.Pin GPIO5_12 = P4_8;
        public const Cpu.Pin GPIO5_13 = P4_9;
        public const Cpu.Pin GPIO5_14 = P4_10;
        public const Cpu.Pin GPIO5_15 = P6_7;
        public const Cpu.Pin GPIO5_16 = P6_8;
        public const Cpu.Pin GPIO5_17 = P9_4;
        public const Cpu.Pin GPIO5_18 = P9_5;
        public const Cpu.Pin GPIO5_19 = PA_4;
        public const Cpu.Pin GPIO5_20 = PB_0;
        public const Cpu.Pin GPIO5_21 = PB_1;
        public const Cpu.Pin GPIO5_22 = PB_2;
        public const Cpu.Pin GPIO5_23 = PB_3;
        public const Cpu.Pin GPIO5_24 = PB_4;
        public const Cpu.Pin GPIO5_25 = PB_5;
        public const Cpu.Pin GPIO5_26 = PB_6;

        public const Cpu.Pin GPIO6_0 = PC_1;
        public const Cpu.Pin GPIO6_1 = PC_2;
        public const Cpu.Pin GPIO6_2 = PC_3;
        public const Cpu.Pin GPIO6_3 = PC_4;
        public const Cpu.Pin GPIO6_4 = PC_5;
        public const Cpu.Pin GPIO6_5 = PC_6;
        public const Cpu.Pin GPIO6_6 = PC_7;
        public const Cpu.Pin GPIO6_7 = PC_8;
        public const Cpu.Pin GPIO6_8 = PC_9;
        public const Cpu.Pin GPIO6_9 = PC_10;
        public const Cpu.Pin GPIO6_10 = PC_11;
        public const Cpu.Pin GPIO6_11 = PC_12;
        public const Cpu.Pin GPIO6_12 = PC_13;
        public const Cpu.Pin GPIO6_13 = PC_14;
        public const Cpu.Pin GPIO6_14 = PD_0;
        public const Cpu.Pin GPIO6_15 = PD_1;
        public const Cpu.Pin GPIO6_16 = PD_2;
        public const Cpu.Pin GPIO6_17 = PD_3;
        public const Cpu.Pin GPIO6_18 = PD_4;
        public const Cpu.Pin GPIO6_19 = PD_5;
        public const Cpu.Pin GPIO6_20 = PD_6;
        public const Cpu.Pin GPIO6_21 = PD_7;
        public const Cpu.Pin GPIO6_22 = PD_8;
        public const Cpu.Pin GPIO6_23 = PD_9;
        public const Cpu.Pin GPIO6_24 = PD_10;
        public const Cpu.Pin GPIO6_25 = PD_11;
        public const Cpu.Pin GPIO6_26 = PD_12;
        public const Cpu.Pin GPIO6_27 = PD_13;
        public const Cpu.Pin GPIO6_28 = PD_14;
        public const Cpu.Pin GPIO6_29 = PD_15;
        public const Cpu.Pin GPIO6_30 = PD_16;

        public const Cpu.Pin GPIO7_0 = PE_0;
        public const Cpu.Pin GPIO7_1 = PE_1;
        public const Cpu.Pin GPIO7_2 = PE_2;
        public const Cpu.Pin GPIO7_3 = PE_3;
        public const Cpu.Pin GPIO7_4 = PE_4;
        public const Cpu.Pin GPIO7_5 = PE_5;
        public const Cpu.Pin GPIO7_6 = PE_5;
        public const Cpu.Pin GPIO7_7 = PE_7;
        public const Cpu.Pin GPIO7_8 = PE_8;
        public const Cpu.Pin GPIO7_9 = PE_9;
        public const Cpu.Pin GPIO7_10 = PE_10;
        public const Cpu.Pin GPIO7_11 = PE_11;
        public const Cpu.Pin GPIO7_12 = PE_12;
        public const Cpu.Pin GPIO7_13 = PE_13;
        public const Cpu.Pin GPIO7_14 = PE_14;
        public const Cpu.Pin GPIO7_15 = PE_15;
        public const Cpu.Pin GPIO7_16 = PF_1;
        public const Cpu.Pin GPIO7_17 = PF_2;
        public const Cpu.Pin GPIO7_18 = PF_3;
        public const Cpu.Pin GPIO7_19 = PF_5;
        public const Cpu.Pin GPIO7_20 = PF_6;
        public const Cpu.Pin GPIO7_21 = PF_7;
        public const Cpu.Pin GPIO7_22 = PF_8;
        public const Cpu.Pin GPIO7_23 = PF_9;
        public const Cpu.Pin GPIO7_24 = PF_10;
        public const Cpu.Pin GPIO7_25 = PF_11;

        //
        public const Cpu.Pin GPIO_NONE = Cpu.Pin.GPIO_NONE;
    }

    public static class SerialPorts
    {
        public const string COM1 = "COM1";
        public const string COM2 = "COM2";
        public const string COM3 = "COM3";
    }
    
    public static class BaudRates
    {
        public const BaudRate Baud9600   = BaudRate.Baudrate9600;
        public const BaudRate Baud19200  = BaudRate.Baudrate19200;
        public const BaudRate Baud38400  = BaudRate.Baudrate38400;
        public const BaudRate Baud57600  = BaudRate.Baudrate57600;
        public const BaudRate Baud115200 = BaudRate.Baudrate115200;
        public const BaudRate Baud230400 = BaudRate.Baudrate230400;
    }

    public static class ResistorModes
    {
        public const Port.ResistorMode PullUp   = Port.ResistorMode.PullUp;
        public const Port.ResistorMode PullDown = Port.ResistorMode.PullDown;
        public const Port.ResistorMode Disabled = Port.ResistorMode.Disabled;
    }

    public static class InterruptModes
    {
        public const Port.InterruptMode InterruptEdgeLow       = Port.InterruptMode.InterruptEdgeLow ;
        public const Port.InterruptMode InterruptEdgeHigh      = Port.InterruptMode.InterruptEdgeHigh;
        public const Port.InterruptMode InterruptEdgeBoth      = Port.InterruptMode.InterruptEdgeBoth;
        public const Port.InterruptMode InterruptEdgeLevelHigh = Port.InterruptMode.InterruptEdgeLevelHigh;
        public const Port.InterruptMode InterruptEdgeLevelLow  = Port.InterruptMode.InterruptEdgeLevelLow;
        public const Port.InterruptMode InterruptNone          = Port.InterruptMode.InterruptNone;
    }

    public static class SPI_Devices
    {
        public const Microsoft.SPOT.Hardware.SPI.SPI_module SPI1 = Microsoft.SPOT.Hardware.SPI.SPI_module.SPI1;
        public const Microsoft.SPOT.Hardware.SPI.SPI_module SPI2 = Microsoft.SPOT.Hardware.SPI.SPI_module.SPI2;
    }
    
}
