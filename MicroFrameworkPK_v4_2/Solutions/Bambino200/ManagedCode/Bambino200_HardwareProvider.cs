////////////////////////////////////////////////////////////////////////////////
// Bambino200HardwareProvider.cs for Micromint Bambino 200
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
// Ported to Micromint Bambino 200 by Micromint USA <support@micromint.com>
////////////////////////////////////////////////////////////////////////////////

using System;
using System.IO.Ports;
using Microsoft.SPOT.Hardware;

namespace Micromint.Hardware.Bambino200
{
    internal class Bambino200HardwareProvider : HardwareProvider
    {
        static Bambino200HardwareProvider()
        {
            Microsoft.SPOT.Hardware.HardwareProvider.Register(new Bambino200HardwareProvider());
        }

        override public void GetSerialPins(string comPort, out Cpu.Pin rxPin, out Cpu.Pin txPin, out Cpu.Pin ctsPin, out Cpu.Pin rtsPin)
        {
            switch (comPort)
            {
                case "COM1":
                    rxPin = Pins.CPU_PIN_P6_5;
                    txPin = Pins.CPU_PIN_P6_4;
                    ctsPin = Pins.GPIO_NONE;
                    rtsPin = Pins.GPIO_NONE;
                    break;
                case "COM2":
                    rxPin = Pins.CPU_PIN_P1_14;
                    txPin = Pins.CPU_PIN_P5_6;
                    ctsPin = Pins.CPU_PIN_P5_4;
                    rtsPin = Pins.CPU_PIN_P4_2;
                    break;
                case "COM3":
                    rxPin = Pins.CPU_PIN_P2_11;
                    txPin = Pins.CPU_PIN_P2_10;
                    ctsPin = Pins.GPIO_NONE;
                    rtsPin = Pins.GPIO_NONE;
                    break;
                default:
                    throw new NotSupportedException();
            }
        }

        override public void GetI2CPins(out Cpu.Pin scl, out Cpu.Pin sda)
        {
            scl = Pins.CPU_PIN_P2_4;
            sda = Pins.CPU_PIN_P2_3;
        }

        override public void GetSpiPins(SPI.SPI_module spi_mod, out Cpu.Pin msk, out Cpu.Pin miso, out Cpu.Pin mosi)
        {
            switch (spi_mod)
            {
                case SPI.SPI_module.SPI1:
                    msk = Pins.CPU_PIN_P3_0;
                    miso = Pins.CPU_PIN_P1_1;
                    mosi = Pins.CPU_PIN_P1_2;
                    break;
                case SPI.SPI_module.SPI2:
                    msk = Pins.CPU_PIN_PF_4;
                    miso = Pins.CPU_PIN_P1_3;
                    mosi = Pins.CPU_PIN_P1_4;
                    break;
                default:
                    throw new NotSupportedException();
            }
        }
    }

    // Specifies identifiers for hardware I/O pins.
    public static class Pins
    {
        // On the LPC43xx the CPU pin name and the GPIO pin name are not the same.
        // To simplify runtime logic, encode SCU and GPIO offsets as a pin identifier
        // using constants.
        //   SCU_OFF(group, num) = ((0x80 * group) + (4 * num));
        //   GPIO_OFF(port, pin) = ((port << 5) + pin);
        //   _pin(group, num, port, pin) = ((SCU_OFF(group,num) << 16) + GPIO_OFF(port,pin));
        //   _pin(group, num, port, pin) =  ((((0x80 * group) + (4 * num)) << 16) + ((port << 5) + pin));

        // CPU Pins
        public const Cpu.Pin CPU_PIN_P0_0 = (Cpu.Pin)((((0x80 * 0x0) + (4 * 0)) << 16) + ((0 << 5) + 0)); // GPIO0[0]
        public const Cpu.Pin CPU_PIN_P0_1 = (Cpu.Pin)((((0x80 * 0x0) + (4 * 1)) << 16) + ((0 << 5) + 1)); // GPIO0[1]

        public const Cpu.Pin CPU_PIN_P1_0 = (Cpu.Pin)((((0x80 * 0x1) + (4 * 0)) << 16) + ((0 << 5) + 4)); // GPIO0[4]
        public const Cpu.Pin CPU_PIN_P1_1 = (Cpu.Pin)((((0x80 * 0x1) + (4 * 1)) << 16) + ((0 << 5) + 8)); // GPIO0[8]
        public const Cpu.Pin CPU_PIN_P1_2 = (Cpu.Pin)((((0x80 * 0x1) + (4 * 2)) << 16) + ((0 << 5) + 9)); // GPIO0[9]
        public const Cpu.Pin CPU_PIN_P1_3 = (Cpu.Pin)((((0x80 * 0x1) + (4 * 3)) << 16) + ((0 << 5) + 10)); // GPIO0[10]
        public const Cpu.Pin CPU_PIN_P1_4 = (Cpu.Pin)((((0x80 * 0x1) + (4 * 4)) << 16) + ((0 << 5) + 11)); // GPIO0[11]
        public const Cpu.Pin CPU_PIN_P1_5 = (Cpu.Pin)((((0x80 * 0x1) + (4 * 5)) << 16) + ((1 << 5) + 8)); // GPIO1[8]
        public const Cpu.Pin CPU_PIN_P1_6 = (Cpu.Pin)((((0x80 * 0x1) + (4 * 6)) << 16) + ((1 << 5) + 9)); // GPIO1[9]
        public const Cpu.Pin CPU_PIN_P1_7 = (Cpu.Pin)((((0x80 * 0x1) + (4 * 7)) << 16) + ((1 << 5) + 0)); // GPIO1[0]
        public const Cpu.Pin CPU_PIN_P1_8 = (Cpu.Pin)((((0x80 * 0x1) + (4 * 8)) << 16) + ((1 << 5) + 1)); // GPIO1[1]
        public const Cpu.Pin CPU_PIN_P1_9 = (Cpu.Pin)((((0x80 * 0x1) + (4 * 9)) << 16) + ((1 << 5) + 2)); // GPIO1[2]
        public const Cpu.Pin CPU_PIN_P1_10 = (Cpu.Pin)((((0x80 * 0x1) + (4 * 10)) << 16) + ((1 << 5) + 3)); // GPIO1[3]
        public const Cpu.Pin CPU_PIN_P1_11 = (Cpu.Pin)((((0x80 * 0x1) + (4 * 11)) << 16) + ((1 << 5) + 4)); // GPIO1[4]
        public const Cpu.Pin CPU_PIN_P1_12 = (Cpu.Pin)((((0x80 * 0x1) + (4 * 12)) << 16) + ((1 << 5) + 5)); // GPIO1[5]
        public const Cpu.Pin CPU_PIN_P1_13 = (Cpu.Pin)((((0x80 * 0x1) + (4 * 13)) << 16) + ((1 << 5) + 6)); // GPIO1[6]
        public const Cpu.Pin CPU_PIN_P1_14 = (Cpu.Pin)((((0x80 * 0x1) + (4 * 14)) << 16) + ((1 << 5) + 7)); // GPIO1[7]
        public const Cpu.Pin CPU_PIN_P1_15 = (Cpu.Pin)((((0x80 * 0x1) + (4 * 15)) << 16) + ((0 << 5) + 2)); // GPIO0[2]
        public const Cpu.Pin CPU_PIN_P1_16 = (Cpu.Pin)((((0x80 * 0x1) + (4 * 16)) << 16) + ((0 << 5) + 3)); // GPIO0[3]
        public const Cpu.Pin CPU_PIN_P1_17 = (Cpu.Pin)((((0x80 * 0x1) + (4 * 17)) << 16) + ((0 << 5) + 12)); // GPIO0[12] high-drive
        public const Cpu.Pin CPU_PIN_P1_18 = (Cpu.Pin)((((0x80 * 0x1) + (4 * 18)) << 16) + ((0 << 5) + 13)); // GPIO0[13]
        public const Cpu.Pin CPU_PIN_P1_19 = (Cpu.Pin)((((0x80 * 0x1) + (4 * 19)) << 16) + ((15 << 5) + 0));
        public const Cpu.Pin CPU_PIN_P1_20 = (Cpu.Pin)((((0x80 * 0x1) + (4 * 20)) << 16) + ((0 << 5) + 15)); // GPIO0[15]

        public const Cpu.Pin CPU_PIN_P2_0 = (Cpu.Pin)((((0x80 * 0x2) + (4 * 0)) << 16) + ((5 << 5) + 0)); // GPIO5[0]
        public const Cpu.Pin CPU_PIN_P2_1 = (Cpu.Pin)((((0x80 * 0x2) + (4 * 1)) << 16) + ((5 << 5) + 1)); // GPIO5[1]
        public const Cpu.Pin CPU_PIN_P2_2 = (Cpu.Pin)((((0x80 * 0x2) + (4 * 2)) << 16) + ((5 << 5) + 2)); // GPIO5[2]
        public const Cpu.Pin CPU_PIN_P2_3 = (Cpu.Pin)((((0x80 * 0x2) + (4 * 3)) << 16) + ((5 << 5) + 3)); // GPIO5[3]  high-drive
        public const Cpu.Pin CPU_PIN_P2_4 = (Cpu.Pin)((((0x80 * 0x2) + (4 * 4)) << 16) + ((5 << 5) + 4)); // GPIO5[4]  high-drive
        public const Cpu.Pin CPU_PIN_P2_5 = (Cpu.Pin)((((0x80 * 0x2) + (4 * 5)) << 16) + ((5 << 5) + 5)); // GPIO5[5]  high-drive
        public const Cpu.Pin CPU_PIN_P2_6 = (Cpu.Pin)((((0x80 * 0x2) + (4 * 6)) << 16) + ((5 << 5) + 6)); // GPIO5[6]
        public const Cpu.Pin CPU_PIN_P2_7 = (Cpu.Pin)((((0x80 * 0x2) + (4 * 7)) << 16) + ((0 << 5) + 7)); // GPIO0[7]
        public const Cpu.Pin CPU_PIN_P2_8 = (Cpu.Pin)((((0x80 * 0x2) + (4 * 8)) << 16) + ((5 << 5) + 7)); // GPIO5[7]
        public const Cpu.Pin CPU_PIN_P2_9 = (Cpu.Pin)((((0x80 * 0x2) + (4 * 9)) << 16) + ((1 << 5) + 10)); // GPIO1[10]
        public const Cpu.Pin CPU_PIN_P2_10 = (Cpu.Pin)((((0x80 * 0x2) + (4 * 10)) << 16) + ((0 << 5) + 14)); // GPIO0[14]
        public const Cpu.Pin CPU_PIN_P2_11 = (Cpu.Pin)((((0x80 * 0x2) + (4 * 11)) << 16) + ((1 << 5) + 11)); // GPIO1[11]
        public const Cpu.Pin CPU_PIN_P2_12 = (Cpu.Pin)((((0x80 * 0x2) + (4 * 12)) << 16) + ((1 << 5) + 12)); // GPIO1[12]
        public const Cpu.Pin CPU_PIN_P2_13 = (Cpu.Pin)((((0x80 * 0x2) + (4 * 13)) << 16) + ((1 << 5) + 13)); // GPIO1[13]

        public const Cpu.Pin CPU_PIN_P3_0 = (Cpu.Pin)((((0x80 * 0x3) + (4 * 0)) << 16) + ((15 << 5) + 0));
        public const Cpu.Pin CPU_PIN_P3_1 = (Cpu.Pin)((((0x80 * 0x3) + (4 * 1)) << 16) + ((5 << 5) + 8)); // GPIO5[8]
        public const Cpu.Pin CPU_PIN_P3_2 = (Cpu.Pin)((((0x80 * 0x3) + (4 * 2)) << 16) + ((5 << 5) + 9)); // GPIO5[9]
        public const Cpu.Pin CPU_PIN_P3_3 = (Cpu.Pin)((((0x80 * 0x3) + (4 * 3)) << 16) + ((15 << 5) + 0));
        public const Cpu.Pin CPU_PIN_P3_4 = (Cpu.Pin)((((0x80 * 0x3) + (4 * 4)) << 16) + ((1 << 5) + 14)); // GPIO1[14]
        public const Cpu.Pin CPU_PIN_P3_5 = (Cpu.Pin)((((0x80 * 0x3) + (4 * 5)) << 16) + ((1 << 5) + 15)); // GPIO1[15]
        public const Cpu.Pin CPU_PIN_P3_6 = (Cpu.Pin)((((0x80 * 0x3) + (4 * 6)) << 16) + ((0 << 5) + 6)); // GPIO0[6]
        public const Cpu.Pin CPU_PIN_P3_7 = (Cpu.Pin)((((0x80 * 0x3) + (4 * 7)) << 16) + ((5 << 5) + 10)); // GPIO5[10]
        public const Cpu.Pin CPU_PIN_P3_8 = (Cpu.Pin)((((0x80 * 0x3) + (4 * 8)) << 16) + ((5 << 5) + 11)); // GPIO5[11]

        public const Cpu.Pin CPU_PIN_P4_0 = (Cpu.Pin)((((0x80 * 0x4) + (4 * 0)) << 16) + ((2 << 5) + 0)); // GPIO2[0]
        public const Cpu.Pin CPU_PIN_P4_1 = (Cpu.Pin)((((0x80 * 0x4) + (4 * 1)) << 16) + ((2 << 5) + 1)); // GPIO2[1]
        public const Cpu.Pin CPU_PIN_P4_2 = (Cpu.Pin)((((0x80 * 0x4) + (4 * 2)) << 16) + ((2 << 5) + 2)); // GPIO2[2]
        public const Cpu.Pin CPU_PIN_P4_3 = (Cpu.Pin)((((0x80 * 0x4) + (4 * 3)) << 16) + ((2 << 5) + 3)); // GPIO2[3]
        public const Cpu.Pin CPU_PIN_P4_4 = (Cpu.Pin)((((0x80 * 0x4) + (4 * 4)) << 16) + ((2 << 5) + 4)); // GPIO2[4]
        public const Cpu.Pin CPU_PIN_P4_5 = (Cpu.Pin)((((0x80 * 0x4) + (4 * 5)) << 16) + ((2 << 5) + 5)); // GPIO2[5]
        public const Cpu.Pin CPU_PIN_P4_6 = (Cpu.Pin)((((0x80 * 0x4) + (4 * 6)) << 16) + ((2 << 5) + 6)); // GPIO2[6]
        public const Cpu.Pin CPU_PIN_P4_7 = (Cpu.Pin)((((0x80 * 0x4) + (4 * 7)) << 16) + ((15 << 5) + 0));
        public const Cpu.Pin CPU_PIN_P4_8 = (Cpu.Pin)((((0x80 * 0x4) + (4 * 8)) << 16) + ((5 << 5) + 12)); // GPIO5[12]
        public const Cpu.Pin CPU_PIN_P4_9 = (Cpu.Pin)((((0x80 * 0x4) + (4 * 9)) << 16) + ((5 << 5) + 13)); // GPIO5[13]
        public const Cpu.Pin CPU_PIN_P4_10 = (Cpu.Pin)((((0x80 * 0x4) + (4 * 10)) << 16) + ((5 << 5) + 14)); // GPIO5[14]

        public const Cpu.Pin CPU_PIN_P5_0 = (Cpu.Pin)((((0x80 * 0x5) + (4 * 0)) << 16) + ((2 << 5) + 9)); // GPIO2[9]
        public const Cpu.Pin CPU_PIN_P5_1 = (Cpu.Pin)((((0x80 * 0x5) + (4 * 1)) << 16) + ((2 << 5) + 10)); // GPIO2[10]
        public const Cpu.Pin CPU_PIN_P5_2 = (Cpu.Pin)((((0x80 * 0x5) + (4 * 2)) << 16) + ((2 << 5) + 11)); // GPIO2[11]
        public const Cpu.Pin CPU_PIN_P5_3 = (Cpu.Pin)((((0x80 * 0x5) + (4 * 3)) << 16) + ((2 << 5) + 12)); // GPIO2[12]
        public const Cpu.Pin CPU_PIN_P5_4 = (Cpu.Pin)((((0x80 * 0x5) + (4 * 4)) << 16) + ((2 << 5) + 13)); // GPIO2[13]
        public const Cpu.Pin CPU_PIN_P5_5 = (Cpu.Pin)((((0x80 * 0x5) + (4 * 5)) << 16) + ((2 << 5) + 14)); // GPIO2[14]
        public const Cpu.Pin CPU_PIN_P5_6 = (Cpu.Pin)((((0x80 * 0x5) + (4 * 6)) << 16) + ((2 << 5) + 15)); // GPIO2[15]
        public const Cpu.Pin CPU_PIN_P5_7 = (Cpu.Pin)((((0x80 * 0x5) + (4 * 7)) << 16) + ((2)) << 16) + ((7)); // GPIO2[7]

        public const Cpu.Pin CPU_PIN_P6_0 = (Cpu.Pin)((((0x80 * 0x6) + (4 * 0)) << 16) + ((15 << 5) + 0));
        public const Cpu.Pin CPU_PIN_P6_1 = (Cpu.Pin)((((0x80 * 0x6) + (4 * 1)) << 16) + ((3 << 5) + 0)); // GPIO3[0]
        public const Cpu.Pin CPU_PIN_P6_2 = (Cpu.Pin)((((0x80 * 0x6) + (4 * 2)) << 16) + ((3 << 5) + 1)); // GPIO3[1]
        public const Cpu.Pin CPU_PIN_P6_3 = (Cpu.Pin)((((0x80 * 0x6) + (4 * 3)) << 16) + ((3 << 5) + 2)); // GPIO3[2]
        public const Cpu.Pin CPU_PIN_P6_4 = (Cpu.Pin)((((0x80 * 0x6) + (4 * 4)) << 16) + ((3 << 5) + 3)); // GPIO3[3]
        public const Cpu.Pin CPU_PIN_P6_5 = (Cpu.Pin)((((0x80 * 0x6) + (4 * 5)) << 16) + ((3 << 5) + 4)); // GPIO3[4]
        public const Cpu.Pin CPU_PIN_P6_6 = (Cpu.Pin)((((0x80 * 0x6) + (4 * 6)) << 16) + ((0 << 5) + 5)); // GPIO0[5]
        public const Cpu.Pin CPU_PIN_P6_7 = (Cpu.Pin)((((0x80 * 0x6) + (4 * 7)) << 16) + ((5 << 5) + 15)); // GPIO5[15]
        public const Cpu.Pin CPU_PIN_P6_8 = (Cpu.Pin)((((0x80 * 0x6) + (4 * 8)) << 16) + ((5 << 5) + 16)); // GPIO5[16]
        public const Cpu.Pin CPU_PIN_P6_9 = (Cpu.Pin)((((0x80 * 0x6) + (4 * 9)) << 16) + ((3 << 5) + 5)); // GPIO3[5]
        public const Cpu.Pin CPU_PIN_P6_10 = (Cpu.Pin)((((0x80 * 0x6) + (4 * 10)) << 16) + ((3 << 5) + 6)); // GPIO3[6]
        public const Cpu.Pin CPU_PIN_P6_11 = (Cpu.Pin)((((0x80 * 0x6) + (4 * 11)) << 16) + ((3 << 5) + 7)); // GPIO3[7]
        public const Cpu.Pin CPU_PIN_P6_12 = (Cpu.Pin)((((0x80 * 0x6) + (4 * 12)) << 16) + ((2 << 5) + 8)); // GPIO2[8]

        public const Cpu.Pin CPU_PIN_P7_0 = (Cpu.Pin)((((0x80 * 0x7) + (4 * 0)) << 16) + ((3 << 5) + 8)); // GPIO3[8]
        public const Cpu.Pin CPU_PIN_P7_1 = (Cpu.Pin)((((0x80 * 0x7) + (4 * 1)) << 16) + ((3 << 5) + 9)); // GPIO3[9]
        public const Cpu.Pin CPU_PIN_P7_2 = (Cpu.Pin)((((0x80 * 0x7) + (4 * 2)) << 16) + ((3 << 5) + 10)); // GPIO3[10]
        public const Cpu.Pin CPU_PIN_P7_3 = (Cpu.Pin)((((0x80 * 0x7) + (4 * 3)) << 16) + ((3 << 5) + 11)); // GPIO3[11]
        public const Cpu.Pin CPU_PIN_P7_4 = (Cpu.Pin)((((0x80 * 0x7) + (4 * 4)) << 16) + ((3 << 5) + 12)); // GPIO3[12]
        public const Cpu.Pin CPU_PIN_P7_5 = (Cpu.Pin)((((0x80 * 0x7) + (4 * 5)) << 16) + ((3 << 5) + 13)); // GPIO3[13]
        public const Cpu.Pin CPU_PIN_P7_6 = (Cpu.Pin)((((0x80 * 0x7) + (4 * 6)) << 16) + ((3 << 5) + 14)); // GPIO3[14]
        public const Cpu.Pin CPU_PIN_P7_7 = (Cpu.Pin)((((0x80 * 0x7) + (4 * 7)) << 16) + ((3 << 5) + 15)); // GPIO3[15]

        public const Cpu.Pin CPU_PIN_P8_0 = (Cpu.Pin)((((0x80 * 0x8) + (4 * 8)) << 16) + ((4 << 5) + 0)); // GPIO4[0]  high-drive
        public const Cpu.Pin CPU_PIN_P8_1 = (Cpu.Pin)((((0x80 * 0x9) + (4 * 0)) << 16) + ((4 << 5) + 1)); // GPIO4[1]  high-drive
        public const Cpu.Pin CPU_PIN_P8_2 = (Cpu.Pin)((((0x80 * 0x9) + (4 * 1)) << 16) + ((4 << 5) + 2)); // GPIO4[2]  high-drive
        public const Cpu.Pin CPU_PIN_P8_3 = (Cpu.Pin)((((0x80 * 0x9) + (4 * 2)) << 16) + ((4 << 5) + 3)); // GPIO4[3]
        public const Cpu.Pin CPU_PIN_P8_4 = (Cpu.Pin)((((0x80 * 0x8) + (4 * 4)) << 16) + ((4 << 5) + 4)); // GPIO4[4]
        public const Cpu.Pin CPU_PIN_P8_5 = (Cpu.Pin)((((0x80 * 0x8) + (4 * 5)) << 16) + ((4 << 5) + 5)); // GPIO4[5]
        public const Cpu.Pin CPU_PIN_P8_6 = (Cpu.Pin)((((0x80 * 0x8) + (4 * 6)) << 16) + ((4 << 5) + 6)); // GPIO4[6]
        public const Cpu.Pin CPU_PIN_P8_7 = (Cpu.Pin)((((0x80 * 0x8) + (4 * 7)) << 16) + ((4 << 5) + 7)); // GPIO4[7]
        public const Cpu.Pin CPU_PIN_P8_8 = (Cpu.Pin)((((0x80 * 0x8) + (4 * 8)) << 16) + ((15 << 5) + 0));

        public const Cpu.Pin CPU_PIN_P9_0 = (Cpu.Pin)((((0x80 * 0x9) + (4 * 0)) << 16) + ((4 << 5) + 12)); // GPIO4[12]
        public const Cpu.Pin CPU_PIN_P9_1 = (Cpu.Pin)((((0x80 * 0x9) + (4 * 1)) << 16) + ((4 << 5) + 13)); // GPIO4[13]
        public const Cpu.Pin CPU_PIN_P9_2 = (Cpu.Pin)((((0x80 * 0x9) + (4 * 2)) << 16) + ((4 << 5) + 14)); // GPIO4[14]
        public const Cpu.Pin CPU_PIN_P9_3 = (Cpu.Pin)((((0x80 * 0x9) + (4 * 3)) << 16) + ((4 << 5) + 15)); // GPIO4[15]
        public const Cpu.Pin CPU_PIN_P9_4 = (Cpu.Pin)((((0x80 * 0x9) + (4 * 4)) << 16) + ((5 << 5) + 17)); // GPIO5[17]
        public const Cpu.Pin CPU_PIN_P9_5 = (Cpu.Pin)((((0x80 * 0x9) + (4 * 5)) << 16) + ((5 << 5) + 18)); // GPIO5[18]
        public const Cpu.Pin CPU_PIN_P9_6 = (Cpu.Pin)((((0x80 * 0x9) + (4 * 6)) << 16) + ((4 << 5) + 11)); // GPIO4[11]

        public const Cpu.Pin CPU_PIN_PA_0 = (Cpu.Pin)((((0x80 * 0xA) + (4 * 0)) << 16) + ((15 << 5) + 0));
        public const Cpu.Pin CPU_PIN_PA_1 = (Cpu.Pin)((((0x80 * 0xA) + (4 * 1)) << 16) + ((4 << 5) + 8)); // GPIO4[8]  high-drive
        public const Cpu.Pin CPU_PIN_PA_2 = (Cpu.Pin)((((0x80 * 0xA) + (4 * 2)) << 16) + ((4 << 5) + 9)); // GPIO4[9]  high-drive
        public const Cpu.Pin CPU_PIN_PA_3 = (Cpu.Pin)((((0x80 * 0xA) + (4 * 3)) << 16) + ((4 << 5) + 10)); // GPIO4[10] high-drive
        public const Cpu.Pin CPU_PIN_PA_4 = (Cpu.Pin)((((0x80 * 0xA) + (4 * 4)) << 16) + ((5 << 5) + 19)); // GPIO5[19]

        public const Cpu.Pin CPU_PIN_PB_0 = (Cpu.Pin)((((0x80 * 0xB) + (4 * 0)) << 16) + ((5 << 5) + 20)); // GPIO5[20]
        public const Cpu.Pin CPU_PIN_PB_1 = (Cpu.Pin)((((0x80 * 0xB) + (4 * 1)) << 16) + ((5 << 5) + 21)); // GPIO5[21]
        public const Cpu.Pin CPU_PIN_PB_2 = (Cpu.Pin)((((0x80 * 0xB) + (4 * 2)) << 16) + ((5 << 5) + 22)); // GPIO5[22]
        public const Cpu.Pin CPU_PIN_PB_3 = (Cpu.Pin)((((0x80 * 0xB) + (4 * 3)) << 16) + ((5 << 5) + 23)); // GPIO5[23]
        public const Cpu.Pin CPU_PIN_PB_4 = (Cpu.Pin)((((0x80 * 0xB) + (4 * 4)) << 16) + ((5 << 5) + 24)); // GPIO5[24]
        public const Cpu.Pin CPU_PIN_PB_5 = (Cpu.Pin)((((0x80 * 0xB) + (4 * 5)) << 16) + ((5 << 5) + 25)); // GPIO5[25]
        public const Cpu.Pin CPU_PIN_PB_6 = (Cpu.Pin)((((0x80 * 0xB) + (4 * 6)) << 16) + ((5 << 5) + 26)); // GPIO5[26]

        public const Cpu.Pin CPU_PIN_PC_0 = (Cpu.Pin)((((0x80 * 0xC) + (4 * 0)) << 16) + ((15 << 5) + 0));
        public const Cpu.Pin CPU_PIN_PC_1 = (Cpu.Pin)((((0x80 * 0xC) + (4 * 1)) << 16) + ((6 << 5) + 0)); // GPIO6[0]
        public const Cpu.Pin CPU_PIN_PC_2 = (Cpu.Pin)((((0x80 * 0xC) + (4 * 2)) << 16) + ((6 << 5) + 1)); // GPIO6[1]
        public const Cpu.Pin CPU_PIN_PC_3 = (Cpu.Pin)((((0x80 * 0xC) + (4 * 3)) << 16) + ((6 << 5) + 2)); // GPIO6[2]
        public const Cpu.Pin CPU_PIN_PC_4 = (Cpu.Pin)((((0x80 * 0xC) + (4 * 4)) << 16) + ((6 << 5) + 3)); // GPIO6[3]
        public const Cpu.Pin CPU_PIN_PC_5 = (Cpu.Pin)((((0x80 * 0xC) + (4 * 5)) << 16) + ((6 << 5) + 4)); // GPIO6[4]
        public const Cpu.Pin CPU_PIN_PC_6 = (Cpu.Pin)((((0x80 * 0xC) + (4 * 6)) << 16) + ((6 << 5) + 5)); // GPIO6[5]
        public const Cpu.Pin CPU_PIN_PC_7 = (Cpu.Pin)((((0x80 * 0xC) + (4 * 7)) << 16) + ((6 << 5) + 6)); // GPIO6[6]
        public const Cpu.Pin CPU_PIN_PC_8 = (Cpu.Pin)((((0x80 * 0xC) + (4 * 8)) << 16) + ((6 << 5) + 7)); // GPIO6[7]
        public const Cpu.Pin CPU_PIN_PC_9 = (Cpu.Pin)((((0x80 * 0xC) + (4 * 9)) << 16) + ((6 << 5) + 8)); // GPIO6[8]
        public const Cpu.Pin CPU_PIN_PC_10 = (Cpu.Pin)((((0x80 * 0xC) + (4 * 10)) << 16) + ((6 << 5) + 9)); // GPIO6[9]
        public const Cpu.Pin CPU_PIN_PC_11 = (Cpu.Pin)((((0x80 * 0xC) + (4 * 11)) << 16) + ((6 << 5) + 10)); // GPIO6[10]
        public const Cpu.Pin CPU_PIN_PC_12 = (Cpu.Pin)((((0x80 * 0xC) + (4 * 12)) << 16) + ((6 << 5) + 11)); // GPIO6[11]
        public const Cpu.Pin CPU_PIN_PC_13 = (Cpu.Pin)((((0x80 * 0xC) + (4 * 13)) << 16) + ((6 << 5) + 12)); // GPIO6[12]
        public const Cpu.Pin CPU_PIN_PC_14 = (Cpu.Pin)((((0x80 * 0xC) + (4 * 14)) << 16) + ((6 << 5) + 13)); // GPIO6[13]

        public const Cpu.Pin CPU_PIN_PD_0 = (Cpu.Pin)((((0x80 * 0xD) + (4 * 0)) << 16) + ((6 << 5) + 14)); // GPIO6[14]
        public const Cpu.Pin CPU_PIN_PD_1 = (Cpu.Pin)((((0x80 * 0xD) + (4 * 1)) << 16) + ((6 << 5) + 15)); // GPIO6[15]
        public const Cpu.Pin CPU_PIN_PD_2 = (Cpu.Pin)((((0x80 * 0xD) + (4 * 2)) << 16) + ((6 << 5) + 16)); // GPIO6[16]
        public const Cpu.Pin CPU_PIN_PD_3 = (Cpu.Pin)((((0x80 * 0xD) + (4 * 3)) << 16) + ((6 << 5) + 17)); // GPIO6[17]
        public const Cpu.Pin CPU_PIN_PD_4 = (Cpu.Pin)((((0x80 * 0xD) + (4 * 4)) << 16) + ((6 << 5) + 18)); // GPIO6[18]
        public const Cpu.Pin CPU_PIN_PD_5 = (Cpu.Pin)((((0x80 * 0xD) + (4 * 5)) << 16) + ((6 << 5) + 19)); // GPIO6[19]
        public const Cpu.Pin CPU_PIN_PD_6 = (Cpu.Pin)((((0x80 * 0xD) + (4 * 6)) << 16) + ((6 << 5) + 20)); // GPIO6[20]
        public const Cpu.Pin CPU_PIN_PD_7 = (Cpu.Pin)((((0x80 * 0xD) + (4 * 7)) << 16) + ((6 << 5) + 21)); // GPIO6[21]
        public const Cpu.Pin CPU_PIN_PD_8 = (Cpu.Pin)((((0x80 * 0xD) + (4 * 8)) << 16) + ((6 << 5) + 22)); // GPIO6[22]
        public const Cpu.Pin CPU_PIN_PD_9 = (Cpu.Pin)((((0x80 * 0xD) + (4 * 9)) << 16) + ((6 << 5) + 23)); // GPIO6[23]
        public const Cpu.Pin CPU_PIN_PD_10 = (Cpu.Pin)((((0x80 * 0xD) + (4 * 10)) << 16) + ((6 << 5) + 24)); // GPIO6[24]
        public const Cpu.Pin CPU_PIN_PD_11 = (Cpu.Pin)((((0x80 * 0xD) + (4 * 11)) << 16) + ((6 << 5) + 25)); // GPIO6[25]
        public const Cpu.Pin CPU_PIN_PD_12 = (Cpu.Pin)((((0x80 * 0xD) + (4 * 12)) << 16) + ((6 << 5) + 26)); // GPIO6[26]
        public const Cpu.Pin CPU_PIN_PD_13 = (Cpu.Pin)((((0x80 * 0xD) + (4 * 13)) << 16) + ((6 << 5) + 27)); // GPIO6[27]
        public const Cpu.Pin CPU_PIN_PD_14 = (Cpu.Pin)((((0x80 * 0xD) + (4 * 14)) << 16) + ((6 << 5) + 28)); // GPIO6[28]
        public const Cpu.Pin CPU_PIN_PD_15 = (Cpu.Pin)((((0x80 * 0xD) + (4 * 15)) << 16) + ((6 << 5) + 29)); // GPIO6[29]
        public const Cpu.Pin CPU_PIN_PD_16 = (Cpu.Pin)((((0x80 * 0xD) + (4 * 16)) << 16) + ((6 << 5) + 30)); // GPIO6[30]

        public const Cpu.Pin CPU_PIN_PE_0 = (Cpu.Pin)((((0x80 * 0xE) + (4 * 0)) << 16) + ((7 << 5) + 0)); // GPIO7[0]
        public const Cpu.Pin CPU_PIN_PE_1 = (Cpu.Pin)((((0x80 * 0xE) + (4 * 1)) << 16) + ((7 << 5) + 1)); // GPIO7[1]
        public const Cpu.Pin CPU_PIN_PE_2 = (Cpu.Pin)((((0x80 * 0xE) + (4 * 2)) << 16) + ((7 << 5) + 2)); // GPIO7[2]
        public const Cpu.Pin CPU_PIN_PE_3 = (Cpu.Pin)((((0x80 * 0xE) + (4 * 3)) << 16) + ((7 << 5) + 3)); // GPIO7[3]
        public const Cpu.Pin CPU_PIN_PE_4 = (Cpu.Pin)((((0x80 * 0xE) + (4 * 4)) << 16) + ((7 << 5) + 4)); // GPIO7[4]
        public const Cpu.Pin CPU_PIN_PE_5 = (Cpu.Pin)((((0x80 * 0xE) + (4 * 5)) << 16) + ((7 << 5) + 5)); // GPIO7[5]
        public const Cpu.Pin CPU_PIN_PE_6 = (Cpu.Pin)((((0x80 * 0xE) + (4 * 6)) << 16) + ((7 << 5) + 6)); // GPIO7[6]
        public const Cpu.Pin CPU_PIN_PE_7 = (Cpu.Pin)((((0x80 * 0xE) + (4 * 7)) << 16) + ((7 << 5) + 7)); // GPIO7[7]
        public const Cpu.Pin CPU_PIN_PE_8 = (Cpu.Pin)((((0x80 * 0xE) + (4 * 8)) << 16) + ((7 << 5) + 8)); // GPIO7[8]
        public const Cpu.Pin CPU_PIN_PE_9 = (Cpu.Pin)((((0x80 * 0xE) + (4 * 9)) << 16) + ((7 << 5) + 9)); // GPIO7[9]
        public const Cpu.Pin CPU_PIN_PE_10 = (Cpu.Pin)((((0x80 * 0xE) + (4 * 10)) << 16) + ((7 << 5) + 10)); // GPIO7[10]
        public const Cpu.Pin CPU_PIN_PE_11 = (Cpu.Pin)((((0x80 * 0xE) + (4 * 11)) << 16) + ((7 << 5) + 11)); // GPIO7[11]
        public const Cpu.Pin CPU_PIN_PE_12 = (Cpu.Pin)((((0x80 * 0xE) + (4 * 12)) << 16) + ((7 << 5) + 12)); // GPIO7[12]
        public const Cpu.Pin CPU_PIN_PE_13 = (Cpu.Pin)((((0x80 * 0xE) + (4 * 13)) << 16) + ((7 << 5) + 13)); // GPIO7[13]
        public const Cpu.Pin CPU_PIN_PE_14 = (Cpu.Pin)((((0x80 * 0xE) + (4 * 14)) << 16) + ((7 << 5) + 14)); // GPIO7[14]
        public const Cpu.Pin CPU_PIN_PE_15 = (Cpu.Pin)((((0x80 * 0xE) + (4 * 15)) << 16) + ((7 << 5) + 15)); // GPIO7[15]

        public const Cpu.Pin CPU_PIN_PF_0 = (Cpu.Pin)((((0x80 * 0xF) + (4 * 0)) << 16) + ((15 << 5) + 0));
        public const Cpu.Pin CPU_PIN_PF_1 = (Cpu.Pin)((((0x80 * 0xF) + (4 * 1)) << 16) + ((7 << 5) + 16)); // GPIO7[16]
        public const Cpu.Pin CPU_PIN_PF_2 = (Cpu.Pin)((((0x80 * 0xF) + (4 * 2)) << 16) + ((7 << 5) + 17)); // GPIO7[17]
        public const Cpu.Pin CPU_PIN_PF_3 = (Cpu.Pin)((((0x80 * 0xF) + (4 * 3)) << 16) + ((7 << 5) + 18)); // GPIO7[18]
        public const Cpu.Pin CPU_PIN_PF_4 = (Cpu.Pin)((((0x80 * 0xF) + (4 * 4)) << 16) + ((15 << 5) + 0));
        public const Cpu.Pin CPU_PIN_PF_5 = (Cpu.Pin)((((0x80 * 0xF) + (4 * 5)) << 16) + ((7 << 5) + 19)); // GPIO7[19]
        public const Cpu.Pin CPU_PIN_PF_6 = (Cpu.Pin)((((0x80 * 0xF) + (4 * 6)) << 16) + ((7 << 5) + 20)); // GPIO7[20]
        public const Cpu.Pin CPU_PIN_PF_7 = (Cpu.Pin)((((0x80 * 0xF) + (4 * 7)) << 16) + ((7 << 5) + 21)); // GPIO7[21]
        public const Cpu.Pin CPU_PIN_PF_8 = (Cpu.Pin)((((0x80 * 0xF) + (4 * 8)) << 16) + ((7 << 5) + 22)); // GPIO7[22]
        public const Cpu.Pin CPU_PIN_PF_9 = (Cpu.Pin)((((0x80 * 0xF) + (4 * 9)) << 16) + ((7 << 5) + 23)); // GPIO7[23]
        public const Cpu.Pin CPU_PIN_PF_10 = (Cpu.Pin)((((0x80 * 0xF) + (4 * 10)) << 16) + ((7 << 5) + 24)); // GPIO7[24]
        public const Cpu.Pin CPU_PIN_PF_11 = (Cpu.Pin)((((0x80 * 0xF) + (4 * 11)) << 16) + ((7 << 5) + 25)); // GPIO7[25]

        // GPIO Pins
        public const Cpu.Pin GPIO_PIN_P0_0 = CPU_PIN_P0_0;
        public const Cpu.Pin GPIO_PIN_P0_1 = CPU_PIN_P0_1 ;
        public const Cpu.Pin GPIO_PIN_P0_2 = CPU_PIN_P1_15;
        public const Cpu.Pin GPIO_PIN_P0_3 = CPU_PIN_P1_16;
        public const Cpu.Pin GPIO_PIN_P0_4 = CPU_PIN_P1_0;
        public const Cpu.Pin GPIO_PIN_P0_5 = CPU_PIN_P6_6;
        public const Cpu.Pin GPIO_PIN_P0_6 = CPU_PIN_P3_6;
        public const Cpu.Pin GPIO_PIN_P0_7 = CPU_PIN_P2_7;
        public const Cpu.Pin GPIO_PIN_P0_8 = CPU_PIN_P1_1;
        public const Cpu.Pin GPIO_PIN_P0_9 = CPU_PIN_P1_2;
        public const Cpu.Pin GPIO_PIN_P0_10 = CPU_PIN_P1_3;
        public const Cpu.Pin GPIO_PIN_P0_11 = CPU_PIN_P1_4;
        public const Cpu.Pin GPIO_PIN_P0_12 = CPU_PIN_P1_17;
        public const Cpu.Pin GPIO_PIN_P0_13 = CPU_PIN_P1_18;
        public const Cpu.Pin GPIO_PIN_P0_14 = CPU_PIN_P2_10;
        public const Cpu.Pin GPIO_PIN_P0_15 = CPU_PIN_P1_20;

        public const Cpu.Pin GPIO_PIN_P1_0 = CPU_PIN_P1_7;
        public const Cpu.Pin GPIO_PIN_P1_1 = CPU_PIN_P1_8;
        public const Cpu.Pin GPIO_PIN_P1_2 = CPU_PIN_P1_9;
        public const Cpu.Pin GPIO_PIN_P1_3 = CPU_PIN_P1_10;
        public const Cpu.Pin GPIO_PIN_P1_4 = CPU_PIN_P1_11;
        public const Cpu.Pin GPIO_PIN_P1_5 = CPU_PIN_P1_12;
        public const Cpu.Pin GPIO_PIN_P1_6 = CPU_PIN_P1_13;
        public const Cpu.Pin GPIO_PIN_P1_7 = CPU_PIN_P1_14;
        public const Cpu.Pin GPIO_PIN_P1_8 = CPU_PIN_P1_5;
        public const Cpu.Pin GPIO_PIN_P1_9 = CPU_PIN_P1_6;
        public const Cpu.Pin GPIO_PIN_P1_10 = CPU_PIN_P2_9;
        public const Cpu.Pin GPIO_PIN_P1_11 = CPU_PIN_P2_11;
        public const Cpu.Pin GPIO_PIN_P1_12 = CPU_PIN_P2_12;
        public const Cpu.Pin GPIO_PIN_P1_13 = CPU_PIN_P2_13;
        public const Cpu.Pin GPIO_PIN_P1_14 = CPU_PIN_P3_4;
        public const Cpu.Pin GPIO_PIN_P1_15 = CPU_PIN_P3_5;

        public const Cpu.Pin GPIO_PIN_P2_0 = CPU_PIN_P4_0;
        public const Cpu.Pin GPIO_PIN_P2_1 = CPU_PIN_P4_1;
        public const Cpu.Pin GPIO_PIN_P2_2 = CPU_PIN_P4_2;
        public const Cpu.Pin GPIO_PIN_P2_3 = CPU_PIN_P4_3;
        public const Cpu.Pin GPIO_PIN_P2_4 = CPU_PIN_P4_4;
        public const Cpu.Pin GPIO_PIN_P2_5 = CPU_PIN_P4_5;
        public const Cpu.Pin GPIO_PIN_P2_6 = CPU_PIN_P4_6;
        public const Cpu.Pin GPIO_PIN_P2_7 = CPU_PIN_P5_7;
        public const Cpu.Pin GPIO_PIN_P2_8 = CPU_PIN_P6_12;
        public const Cpu.Pin GPIO_PIN_P2_9 = CPU_PIN_P5_0;
        public const Cpu.Pin GPIO_PIN_P2_10 = CPU_PIN_P5_1;
        public const Cpu.Pin GPIO_PIN_P2_11 = CPU_PIN_P5_2;
        public const Cpu.Pin GPIO_PIN_P2_12 = CPU_PIN_P5_3;
        public const Cpu.Pin GPIO_PIN_P2_13 = CPU_PIN_P5_4;
        public const Cpu.Pin GPIO_PIN_P2_14 = CPU_PIN_P5_5;
        public const Cpu.Pin GPIO_PIN_P2_15 = CPU_PIN_P5_6;

        public const Cpu.Pin GPIO_PIN_P3_0 = CPU_PIN_P6_1;
        public const Cpu.Pin GPIO_PIN_P3_1 = CPU_PIN_P6_2;
        public const Cpu.Pin GPIO_PIN_P3_2 = CPU_PIN_P6_3;
        public const Cpu.Pin GPIO_PIN_P3_3 = CPU_PIN_P6_4;
        public const Cpu.Pin GPIO_PIN_P3_4 = CPU_PIN_P6_5;
        public const Cpu.Pin GPIO_PIN_P3_5 = CPU_PIN_P6_9;
        public const Cpu.Pin GPIO_PIN_P3_6 = CPU_PIN_P6_10;
        public const Cpu.Pin GPIO_PIN_P3_7 = CPU_PIN_P6_11;
        public const Cpu.Pin GPIO_PIN_P3_8 = CPU_PIN_P7_0;
        public const Cpu.Pin GPIO_PIN_P3_9 = CPU_PIN_P7_1;
        public const Cpu.Pin GPIO_PIN_P3_10 = CPU_PIN_P7_2;
        public const Cpu.Pin GPIO_PIN_P3_11 = CPU_PIN_P7_3;
        public const Cpu.Pin GPIO_PIN_P3_12 = CPU_PIN_P7_4;
        public const Cpu.Pin GPIO_PIN_P3_13 = CPU_PIN_P7_5;
        public const Cpu.Pin GPIO_PIN_P3_14 = CPU_PIN_P7_6;
        public const Cpu.Pin GPIO_PIN_P3_15 = CPU_PIN_P7_7;

        public const Cpu.Pin GPIO_PIN_P4_0 = CPU_PIN_P8_0;
        public const Cpu.Pin GPIO_PIN_P4_1 = CPU_PIN_P8_1;
        public const Cpu.Pin GPIO_PIN_P4_2 = CPU_PIN_P8_2;
        public const Cpu.Pin GPIO_PIN_P4_3 = CPU_PIN_P8_3;
        public const Cpu.Pin GPIO_PIN_P4_4 = CPU_PIN_P8_4;
        public const Cpu.Pin GPIO_PIN_P4_5 = CPU_PIN_P8_5;
        public const Cpu.Pin GPIO_PIN_P4_6 = CPU_PIN_P8_6;
        public const Cpu.Pin GPIO_PIN_P4_7 = CPU_PIN_P8_7;
        public const Cpu.Pin GPIO_PIN_P4_8 = CPU_PIN_PA_1;
        public const Cpu.Pin GPIO_PIN_P4_9 = CPU_PIN_PA_2;
        public const Cpu.Pin GPIO_PIN_P4_10 = CPU_PIN_PA_3;
        public const Cpu.Pin GPIO_PIN_P4_11 = CPU_PIN_P9_6;
        public const Cpu.Pin GPIO_PIN_P4_12 = CPU_PIN_P9_0;
        public const Cpu.Pin GPIO_PIN_P4_13 = CPU_PIN_P9_1;
        public const Cpu.Pin GPIO_PIN_P4_14 = CPU_PIN_P9_2;
        public const Cpu.Pin GPIO_PIN_P4_15 = CPU_PIN_P9_3;

        public const Cpu.Pin GPIO_PIN_P5_0 = CPU_PIN_P2_0;
        public const Cpu.Pin GPIO_PIN_P5_1 = CPU_PIN_P2_1;
        public const Cpu.Pin GPIO_PIN_P5_2 = CPU_PIN_P2_2;
        public const Cpu.Pin GPIO_PIN_P5_3 = CPU_PIN_P2_3;
        public const Cpu.Pin GPIO_PIN_P5_4 = CPU_PIN_P2_4;
        public const Cpu.Pin GPIO_PIN_P5_5 = CPU_PIN_P2_5;
        public const Cpu.Pin GPIO_PIN_P5_6 = CPU_PIN_P2_6;
        public const Cpu.Pin GPIO_PIN_P5_7 = CPU_PIN_P2_8;
        public const Cpu.Pin GPIO_PIN_P5_8 = CPU_PIN_P3_1;
        public const Cpu.Pin GPIO_PIN_P5_9 = CPU_PIN_P3_2;
        public const Cpu.Pin GPIO_PIN_P5_10 = CPU_PIN_P3_7;
        public const Cpu.Pin GPIO_PIN_P5_11 = CPU_PIN_P3_8;
        public const Cpu.Pin GPIO_PIN_P5_12 = CPU_PIN_P4_8;
        public const Cpu.Pin GPIO_PIN_P5_13 = CPU_PIN_P4_9;
        public const Cpu.Pin GPIO_PIN_P5_14 = CPU_PIN_P4_10;
        public const Cpu.Pin GPIO_PIN_P5_15 = CPU_PIN_P6_7;
        public const Cpu.Pin GPIO_PIN_P5_16 = CPU_PIN_P6_8;
        public const Cpu.Pin GPIO_PIN_P5_17 = CPU_PIN_P9_4;
        public const Cpu.Pin GPIO_PIN_P5_18 = CPU_PIN_P9_5;
        public const Cpu.Pin GPIO_PIN_P5_19 = CPU_PIN_PA_4;
        public const Cpu.Pin GPIO_PIN_P5_20 = CPU_PIN_PB_0;
        public const Cpu.Pin GPIO_PIN_P5_21 = CPU_PIN_PB_1;
        public const Cpu.Pin GPIO_PIN_P5_22 = CPU_PIN_PB_2;
        public const Cpu.Pin GPIO_PIN_P5_23 = CPU_PIN_PB_3;
        public const Cpu.Pin GPIO_PIN_P5_24 = CPU_PIN_PB_4;
        public const Cpu.Pin GPIO_PIN_P5_25 = CPU_PIN_PB_5;
        public const Cpu.Pin GPIO_PIN_P5_26 = CPU_PIN_PB_6;

        public const Cpu.Pin GPIO_PIN_P6_0 = CPU_PIN_PC_1;
        public const Cpu.Pin GPIO_PIN_P6_1 = CPU_PIN_PC_2;
        public const Cpu.Pin GPIO_PIN_P6_2 = CPU_PIN_PC_3;
        public const Cpu.Pin GPIO_PIN_P6_3 = CPU_PIN_PC_4;
        public const Cpu.Pin GPIO_PIN_P6_4 = CPU_PIN_PC_5;
        public const Cpu.Pin GPIO_PIN_P6_5 = CPU_PIN_PC_6;
        public const Cpu.Pin GPIO_PIN_P6_6 = CPU_PIN_PC_7;
        public const Cpu.Pin GPIO_PIN_P6_7 = CPU_PIN_PC_8;
        public const Cpu.Pin GPIO_PIN_P6_8 = CPU_PIN_PC_9;
        public const Cpu.Pin GPIO_PIN_P6_9 = CPU_PIN_PC_10;
        public const Cpu.Pin GPIO_PIN_P6_10 = CPU_PIN_PC_11;
        public const Cpu.Pin GPIO_PIN_P6_11 = CPU_PIN_PC_12;
        public const Cpu.Pin GPIO_PIN_P6_12 = CPU_PIN_PC_13;
        public const Cpu.Pin GPIO_PIN_P6_13 = CPU_PIN_PC_14;
        public const Cpu.Pin GPIO_PIN_P6_14 = CPU_PIN_PD_0;
        public const Cpu.Pin GPIO_PIN_P6_15 = CPU_PIN_PD_1;
        public const Cpu.Pin GPIO_PIN_P6_16 = CPU_PIN_PD_2;
        public const Cpu.Pin GPIO_PIN_P6_17 = CPU_PIN_PD_3;
        public const Cpu.Pin GPIO_PIN_P6_18 = CPU_PIN_PD_4;
        public const Cpu.Pin GPIO_PIN_P6_19 = CPU_PIN_PD_5;
        public const Cpu.Pin GPIO_PIN_P6_20 = CPU_PIN_PD_6;
        public const Cpu.Pin GPIO_PIN_P6_21 = CPU_PIN_PD_7;
        public const Cpu.Pin GPIO_PIN_P6_22 = CPU_PIN_PD_8;
        public const Cpu.Pin GPIO_PIN_P6_23 = CPU_PIN_PD_9;
        public const Cpu.Pin GPIO_PIN_P6_24 = CPU_PIN_PD_10;
        public const Cpu.Pin GPIO_PIN_P6_25 = CPU_PIN_PD_11;
        public const Cpu.Pin GPIO_PIN_P6_26 = CPU_PIN_PD_12;
        public const Cpu.Pin GPIO_PIN_P6_27 = CPU_PIN_PD_13;
        public const Cpu.Pin GPIO_PIN_P6_28 = CPU_PIN_PD_14;
        public const Cpu.Pin GPIO_PIN_P6_29 = CPU_PIN_PD_15;
        public const Cpu.Pin GPIO_PIN_P6_30 = CPU_PIN_PD_16;

        public const Cpu.Pin GPIO_PIN_P7_0 = CPU_PIN_PE_0;
        public const Cpu.Pin GPIO_PIN_P7_1 = CPU_PIN_PE_1;
        public const Cpu.Pin GPIO_PIN_P7_2 = CPU_PIN_PE_2;
        public const Cpu.Pin GPIO_PIN_P7_3 = CPU_PIN_PE_3;
        public const Cpu.Pin GPIO_PIN_P7_4 = CPU_PIN_PE_4;
        public const Cpu.Pin GPIO_PIN_P7_5 = CPU_PIN_PE_5;
        public const Cpu.Pin GPIO_PIN_P7_6 = CPU_PIN_PE_5;
        public const Cpu.Pin GPIO_PIN_P7_7 = CPU_PIN_PE_7;
        public const Cpu.Pin GPIO_PIN_P7_8 = CPU_PIN_PE_8;
        public const Cpu.Pin GPIO_PIN_P7_9 = CPU_PIN_PE_9;
        public const Cpu.Pin GPIO_PIN_P7_10 = CPU_PIN_PE_10;
        public const Cpu.Pin GPIO_PIN_P7_11 = CPU_PIN_PE_11;
        public const Cpu.Pin GPIO_PIN_P7_12 = CPU_PIN_PE_12;
        public const Cpu.Pin GPIO_PIN_P7_13 = CPU_PIN_PE_13;
        public const Cpu.Pin GPIO_PIN_P7_14 = CPU_PIN_PE_14;
        public const Cpu.Pin GPIO_PIN_P7_15 = CPU_PIN_PE_15;
        public const Cpu.Pin GPIO_PIN_P7_16 = CPU_PIN_PF_1;
        public const Cpu.Pin GPIO_PIN_P7_17 = CPU_PIN_PF_2;
        public const Cpu.Pin GPIO_PIN_P7_18 = CPU_PIN_PF_3;
        public const Cpu.Pin GPIO_PIN_P7_19 = CPU_PIN_PF_5;
        public const Cpu.Pin GPIO_PIN_P7_20 = CPU_PIN_PF_6;
        public const Cpu.Pin GPIO_PIN_P7_21 = CPU_PIN_PF_7;
        public const Cpu.Pin GPIO_PIN_P7_22 = CPU_PIN_PF_8;
        public const Cpu.Pin GPIO_PIN_P7_23 = CPU_PIN_PF_9;
        public const Cpu.Pin GPIO_PIN_P7_24 = CPU_PIN_PF_10;
        public const Cpu.Pin GPIO_PIN_P7_25 = CPU_PIN_PF_11;

        //
        public const Cpu.Pin GPIO_NONE = Cpu.Pin.GPIO_NONE;
    }

    // Specifies identifiers for analog channels.
    public static class AnalogChannels
    {
        public const Cpu.AnalogChannel ANALOG_PIN_A0 = Cpu.AnalogChannel.ANALOG_0;
        public const Cpu.AnalogChannel ANALOG_PIN_A1 = Cpu.AnalogChannel.ANALOG_1;
        public const Cpu.AnalogChannel ANALOG_PIN_A2 = Cpu.AnalogChannel.ANALOG_2;
        public const Cpu.AnalogChannel ANALOG_PIN_A3 = Cpu.AnalogChannel.ANALOG_3;
        public const Cpu.AnalogChannel ANALOG_PIN_A4 = Cpu.AnalogChannel.ANALOG_4;
        public const Cpu.AnalogChannel ANALOG_PIN_A5 = Cpu.AnalogChannel.ANALOG_5;
        public const Cpu.AnalogChannel ANALOG_NONE = Cpu.AnalogChannel.ANALOG_NONE;
    }

    // Specifies identifiers for pwm channels.
    public static class PWMChannels
    {
        public const Cpu.PWMChannel PWM_PIN_D5 = Cpu.PWMChannel.PWM_0;
        public const Cpu.PWMChannel PWM_PIN_D6 = Cpu.PWMChannel.PWM_1;
        public const Cpu.PWMChannel PWM_PIN_D9 = Cpu.PWMChannel.PWM_2;
        public const Cpu.PWMChannel PWM_PIN_D10 = Cpu.PWMChannel.PWM_3;
        public const Cpu.PWMChannel PWM_NONE = Cpu.PWMChannel.PWM_NONE;
    }

    public static class SerialPorts
    {
        public const string COM1 = "COM1";
        public const string COM2 = "COM2";
        public const string COM3 = "COM3";
    }

    public static class BaudRates
    {
        public const BaudRate Baud2400 = BaudRate.Baudrate2400;
        public const BaudRate Baud9600 = BaudRate.Baudrate9600;
        public const BaudRate Baud19200 = BaudRate.Baudrate19200;
        public const BaudRate Baud38400 = BaudRate.Baudrate38400;
        public const BaudRate Baud57600 = BaudRate.Baudrate57600;
        public const BaudRate Baud115200 = BaudRate.Baudrate115200;
    }

    public static class ResistorModes
    {
        public const Port.ResistorMode PullUp = Port.ResistorMode.PullUp;
        public const Port.ResistorMode Disabled = Port.ResistorMode.Disabled;
    }

    public static class InterruptModes
    {
        public const Port.InterruptMode InterruptEdgeLow = Port.InterruptMode.InterruptEdgeLow;
        public const Port.InterruptMode InterruptEdgeHigh = Port.InterruptMode.InterruptEdgeHigh;
        public const Port.InterruptMode InterruptEdgeBoth = Port.InterruptMode.InterruptEdgeBoth;
        public const Port.InterruptMode InterruptEdgeLevelHigh = Port.InterruptMode.InterruptEdgeLevelHigh;
        public const Port.InterruptMode InterruptEdgeLevelLow = Port.InterruptMode.InterruptEdgeLevelLow;
        public const Port.InterruptMode InterruptNone = Port.InterruptMode.InterruptNone;
    }

    public static class SPI_Devices
    {
        public const Microsoft.SPOT.Hardware.SPI.SPI_module SPI1 = Microsoft.SPOT.Hardware.SPI.SPI_module.SPI1;
        public const Microsoft.SPOT.Hardware.SPI.SPI_module SPI2 = Microsoft.SPOT.Hardware.SPI.SPI_module.SPI2;
    }
}
