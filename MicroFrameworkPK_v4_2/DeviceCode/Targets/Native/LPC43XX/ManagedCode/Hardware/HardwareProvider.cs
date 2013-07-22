////////////////////////////////////////////////////////////////////////////////
// HardwareProvider.cs - Hardware configuration example for NXP LPC43XX boards
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
using System.Threading;
using System.Runtime.CompilerServices;
using Microsoft.SPOT.Hardware;

namespace Microsoft.SPOT.Hardware.LPC43XX
{
    internal class LPC43XXHardwareProvider : HardwareProvider
    {
        static LPC43XXHardwareProvider()
        {
            Microsoft.SPOT.Hardware.HardwareProvider.Register(new LPC43XXHardwareProvider());
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
}
