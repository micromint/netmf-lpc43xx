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
                    rxPin = Pin.P6_5;
                    txPin = Pin.P6_4;
                    ctsPin = Pin.GPIO_NONE;
                    rtsPin = Pin.GPIO_NONE;
                    break;
                case "COM2":
                    rxPin = Pin.P1_14;
                    txPin = Pin.P5_6;
                    ctsPin = Pin.P5_4;
                    rtsPin = Pin.P4_2;
                    break;
                case "COM3":
                    rxPin = Pin.P2_11;
                    txPin = Pin.P2_10;
                    ctsPin = Pin.GPIO_NONE;
                    rtsPin = Pin.GPIO_NONE;
                    break;
                default:
                    throw new NotSupportedException();
            }
        }

        override public void GetI2CPins(out Cpu.Pin scl, out Cpu.Pin sda)
        {
            scl = Pin.P2_4;
            sda = Pin.P2_3;
        }

        override public void GetSpiPins(SPI.SPI_module spi_mod, out Cpu.Pin msk, out Cpu.Pin miso, out Cpu.Pin mosi)
        {
            switch (spi_mod)
            {
                case SPI.SPI_module.SPI1:
                    msk = Pin.P3_0;
                    miso = Pin.P1_1;
                    mosi = Pin.P1_2;
                    break;
                case SPI.SPI_module.SPI2:
                    msk = Pin.PF_4;
                    miso = Pin.P1_3;
                    mosi = Pin.P1_4;
                    break;
                default:
                    throw new NotSupportedException();
            }
        }
    }

    // Specifies identifiers for hardware I/O Pin.
    public static class Pin
    {
        // On the LPC43xx the CPU pin name and the GPIO pin name are not the same.
        // To simplify runtime logic, encode SCU and GPIO offsets as a pin identifier
        // using constants.
        //   SCU_OFF(group, num) = ((0x80 * group) + (4 * num));
        //   GPIO_OFF(port, pin) = ((port << 5) + pin);
        //   netmf _pin(group, num, port, pin) = ((SCU_OFF(group,num) << 16)
        //                                       + GPIO_OFF(port,pin));
        //   netmf_pin(group, num, port, pin) =  ((((0x80 * group) + (4 * num)) << 16)
        //                                       + ((port << 5) + pin));
        // Example:  P5_0  GPIO2[9]
        //   P5_0 = (Cpu.Pin)((((0x80 * 0x5) + (4 * 0)) << 16)
        //                  + ((2 << 5) + 9)); // GPIO2[9]
        //   GPIO2_9 = P5_0;

        // CPU Pins
        public const Cpu.Pin P0_0 = (Cpu.Pin) 0x0000000; // GPIO0[0]
        public const Cpu.Pin P0_1 = (Cpu.Pin) 0x0040001; // GPIO0[1]

        public const Cpu.Pin P1_0 = (Cpu.Pin) 0x00800004; // GPIO0[4]
        public const Cpu.Pin P1_1 = (Cpu.Pin) 0x00840008; // GPIO0[8]
        public const Cpu.Pin P1_2 = (Cpu.Pin) 0x00880009; // GPIO0[9]
        public const Cpu.Pin P1_3 = (Cpu.Pin) 0x008C000A; // GPIO0[10]
        public const Cpu.Pin P1_4 = (Cpu.Pin) 0x0090000B; // GPIO0[11]
        public const Cpu.Pin P1_5 = (Cpu.Pin) 0x00940028; // GPIO1[8]
        public const Cpu.Pin P1_6 = (Cpu.Pin) 0x00980029; // GPIO1[9]
        public const Cpu.Pin P1_7 = (Cpu.Pin) 0x009C0020; // GPIO1[0]
        public const Cpu.Pin P1_8 = (Cpu.Pin) 0x00A00021; // GPIO1[1]
        public const Cpu.Pin P1_9 = (Cpu.Pin) 0x00A40022; // GPIO1[2]
        public const Cpu.Pin P1_10 = (Cpu.Pin) 0x00A80023; // GPIO1[3]
        public const Cpu.Pin P1_11 = (Cpu.Pin) 0x00AC0024; // GPIO1[4]
        public const Cpu.Pin P1_12 = (Cpu.Pin) 0x00B00025; // GPIO1[5]
        public const Cpu.Pin P1_13 = (Cpu.Pin) 0x00B40026; // GPIO1[6]
        public const Cpu.Pin P1_14 = (Cpu.Pin) 0x00B80027; // GPIO1[7]
        public const Cpu.Pin P1_15 = (Cpu.Pin) 0x00BC0002; // GPIO0[2]
        public const Cpu.Pin P1_16 = (Cpu.Pin) 0x00C00003; // GPIO0[3]
        public const Cpu.Pin P1_17 = (Cpu.Pin) 0x00C4000C; // GPIO0[12] high-drive
        public const Cpu.Pin P1_18 = (Cpu.Pin) 0x00C8000D; // GPIO0[13]
        public const Cpu.Pin P1_19 = (Cpu.Pin) 0x00CC01E0;
        public const Cpu.Pin P1_20 = (Cpu.Pin) 0x00D0000F; // GPIO0[15]

        public const Cpu.Pin P2_0 = (Cpu.Pin) 0x0010000A0; // GPIO5[0]
        public const Cpu.Pin P2_1 = (Cpu.Pin) 0x0010400A1; // GPIO5[1]
        public const Cpu.Pin P2_2 = (Cpu.Pin) 0x0010800A2; // GPIO5[2]
        public const Cpu.Pin P2_3 = (Cpu.Pin) 0x0010C00A3; // GPIO5[3]  high-drive
        public const Cpu.Pin P2_4 = (Cpu.Pin) 0x0011000A4; // GPIO5[4]  high-drive
        public const Cpu.Pin P2_5 = (Cpu.Pin) 0x0011400A5; // GPIO5[5]  high-drive
        public const Cpu.Pin P2_6 = (Cpu.Pin) 0x0011800A6; // GPIO5[6]
        public const Cpu.Pin P2_7 = (Cpu.Pin) 0x0011C0007; // GPIO0[7]
        public const Cpu.Pin P2_8 = (Cpu.Pin) 0x0012000A7; // GPIO5[7]
        public const Cpu.Pin P2_9 = (Cpu.Pin) 0x00124002A; // GPIO1[10]
        public const Cpu.Pin P2_10 = (Cpu.Pin) 0x00128000E; // GPIO0[14]
        public const Cpu.Pin P2_11 = (Cpu.Pin) 0x0012C002B; // GPIO1[11]
        public const Cpu.Pin P2_12 = (Cpu.Pin) 0x00130002C; // GPIO1[12]
        public const Cpu.Pin P2_13 = (Cpu.Pin) 0x00134002D; // GPIO1[13]

        public const Cpu.Pin P3_0 = (Cpu.Pin) 0x0018001E0;
        public const Cpu.Pin P3_1 = (Cpu.Pin) 0x0018400A8; // GPIO5[8]
        public const Cpu.Pin P3_2 = (Cpu.Pin) 0x0018800A9; // GPIO5[9]
        public const Cpu.Pin P3_3 = (Cpu.Pin) 0x0018C01E0;
        public const Cpu.Pin P3_4 = (Cpu.Pin) 0x00190002E; // GPIO1[14]
        public const Cpu.Pin P3_5 = (Cpu.Pin) 0x00194002F; // GPIO1[15]
        public const Cpu.Pin P3_6 = (Cpu.Pin) 0x001980006; // GPIO0[6]
        public const Cpu.Pin P3_7 = (Cpu.Pin) 0x0019C00AA; // GPIO5[10]
        public const Cpu.Pin P3_8 = (Cpu.Pin) 0x001A000AB; // GPIO5[11]

        public const Cpu.Pin P4_0 = (Cpu.Pin) 0x002000040; // GPIO2[0]
        public const Cpu.Pin P4_1 = (Cpu.Pin) 0x002040041; // GPIO2[1]
        public const Cpu.Pin P4_2 = (Cpu.Pin) 0x002080042; // GPIO2[2]
        public const Cpu.Pin P4_3 = (Cpu.Pin) 0x0020C0043; // GPIO2[3]
        public const Cpu.Pin P4_4 = (Cpu.Pin) 0x002100044; // GPIO2[4]
        public const Cpu.Pin P4_5 = (Cpu.Pin) 0x002140045; // GPIO2[5]
        public const Cpu.Pin P4_6 = (Cpu.Pin) 0x002180046; // GPIO2[6]
        public const Cpu.Pin P4_7 = (Cpu.Pin) 0x0021C01E0;
        public const Cpu.Pin P4_8 = (Cpu.Pin) 0x0022000AC; // GPIO5[12]
        public const Cpu.Pin P4_9 = (Cpu.Pin) 0x0022400AD; // GPIO5[13]
        public const Cpu.Pin P4_10 = (Cpu.Pin) 0x0022800AE; // GPIO5[14]

        public const Cpu.Pin P5_0 = (Cpu.Pin) 0x002800049; // GPIO2[9]
        public const Cpu.Pin P5_1 = (Cpu.Pin) 0x00284004A; // GPIO2[10]
        public const Cpu.Pin P5_2 = (Cpu.Pin) 0x00288004B; // GPIO2[11]
        public const Cpu.Pin P5_3 = (Cpu.Pin) 0x0028C004C; // GPIO2[12]
        public const Cpu.Pin P5_4 = (Cpu.Pin) 0x00290004D; // GPIO2[13]
        public const Cpu.Pin P5_5 = (Cpu.Pin) 0x00294004E; // GPIO2[14]
        public const Cpu.Pin P5_6 = (Cpu.Pin) 0x00298004F; // GPIO2[15]
        public const Cpu.Pin P5_7 = (Cpu.Pin) 0x0029E0007; // GPIO2[7]

        public const Cpu.Pin P6_0 = (Cpu.Pin) 0x0030001E0;
        public const Cpu.Pin P6_1 = (Cpu.Pin) 0x003040060; // GPIO3[0]
        public const Cpu.Pin P6_2 = (Cpu.Pin) 0x003080061; // GPIO3[1]
        public const Cpu.Pin P6_3 = (Cpu.Pin) 0x0030C0062; // GPIO3[2]
        public const Cpu.Pin P6_4 = (Cpu.Pin) 0x003100063; // GPIO3[3]
        public const Cpu.Pin P6_5 = (Cpu.Pin) 0x003140064; // GPIO3[4]
        public const Cpu.Pin P6_6 = (Cpu.Pin) 0x003180005; // GPIO0[5]
        public const Cpu.Pin P6_7 = (Cpu.Pin) 0x0031C00AF; // GPIO5[15]
        public const Cpu.Pin P6_8 = (Cpu.Pin) 0x0032000B0; // GPIO5[16]
        public const Cpu.Pin P6_9 = (Cpu.Pin) 0x003240065; // GPIO3[5]
        public const Cpu.Pin P6_10 = (Cpu.Pin) 0x003280066; // GPIO3[6]
        public const Cpu.Pin P6_11 = (Cpu.Pin) 0x0032C0067; // GPIO3[7]
        public const Cpu.Pin P6_12 = (Cpu.Pin) 0x003300048; // GPIO2[8]

        public const Cpu.Pin P7_0 = (Cpu.Pin) 0x003800068; // GPIO3[8]
        public const Cpu.Pin P7_1 = (Cpu.Pin) 0x003840069; // GPIO3[9]
        public const Cpu.Pin P7_2 = (Cpu.Pin) 0x00388006A; // GPIO3[10]
        public const Cpu.Pin P7_3 = (Cpu.Pin) 0x0038C006B; // GPIO3[11]
        public const Cpu.Pin P7_4 = (Cpu.Pin) 0x00390006C; // GPIO3[12]
        public const Cpu.Pin P7_5 = (Cpu.Pin) 0x00394006D; // GPIO3[13]
        public const Cpu.Pin P7_6 = (Cpu.Pin) 0x00398006E; // GPIO3[14]
        public const Cpu.Pin P7_7 = (Cpu.Pin) 0x0039C006F; // GPIO3[15]

        public const Cpu.Pin P8_0 = (Cpu.Pin) 0x004200080; // GPIO4[0]  high-drive
        public const Cpu.Pin P8_1 = (Cpu.Pin) 0x004800081; // GPIO4[1]  high-drive
        public const Cpu.Pin P8_2 = (Cpu.Pin) 0x004840082; // GPIO4[2]  high-drive
        public const Cpu.Pin P8_3 = (Cpu.Pin) 0x004880083; // GPIO4[3]
        public const Cpu.Pin P8_4 = (Cpu.Pin) 0x004100084; // GPIO4[4]
        public const Cpu.Pin P8_5 = (Cpu.Pin) 0x004140085; // GPIO4[5]
        public const Cpu.Pin P8_6 = (Cpu.Pin) 0x004180086; // GPIO4[6]
        public const Cpu.Pin P8_7 = (Cpu.Pin) 0x0041C0087; // GPIO4[7]
        public const Cpu.Pin P8_8 = (Cpu.Pin) 0x0042001E0;

        public const Cpu.Pin P9_0 = (Cpu.Pin) 0x00480008C; // GPIO4[12]
        public const Cpu.Pin P9_1 = (Cpu.Pin) 0x00484008D; // GPIO4[13]
        public const Cpu.Pin P9_2 = (Cpu.Pin) 0x00488008E; // GPIO4[14]
        public const Cpu.Pin P9_3 = (Cpu.Pin) 0x0048C008F; // GPIO4[15]
        public const Cpu.Pin P9_4 = (Cpu.Pin) 0x0049000B1; // GPIO5[17]
        public const Cpu.Pin P9_5 = (Cpu.Pin) 0x0049400B2; // GPIO5[18]
        public const Cpu.Pin P9_6 = (Cpu.Pin) 0x00498008B; // GPIO4[11]

        public const Cpu.Pin PA_0 = (Cpu.Pin) 0x0050001E0;
        public const Cpu.Pin PA_1 = (Cpu.Pin) 0x005040088; // GPIO4[8]  high-drive
        public const Cpu.Pin PA_2 = (Cpu.Pin) 0x005080089; // GPIO4[9]  high-drive
        public const Cpu.Pin PA_3 = (Cpu.Pin) 0x0050C008A; // GPIO4[10] high-drive
        public const Cpu.Pin PA_4 = (Cpu.Pin) 0x0051000B3; // GPIO5[19]

        public const Cpu.Pin PB_0 = (Cpu.Pin) 0x0058000B4; // GPIO5[20]
        public const Cpu.Pin PB_1 = (Cpu.Pin) 0x0058400B5; // GPIO5[21]
        public const Cpu.Pin PB_2 = (Cpu.Pin) 0x0058800B6; // GPIO5[22]
        public const Cpu.Pin PB_3 = (Cpu.Pin) 0x0058C00B7; // GPIO5[23]
        public const Cpu.Pin PB_4 = (Cpu.Pin) 0x0059000B8; // GPIO5[24]
        public const Cpu.Pin PB_5 = (Cpu.Pin) 0x0059400B9; // GPIO5[25]
        public const Cpu.Pin PB_6 = (Cpu.Pin) 0x0059800BA; // GPIO5[26]

        public const Cpu.Pin PC_0 = (Cpu.Pin) 0x0060001E0;
        public const Cpu.Pin PC_1 = (Cpu.Pin) 0x0060400C0; // GPIO6[0]
        public const Cpu.Pin PC_2 = (Cpu.Pin) 0x0060800C1; // GPIO6[1]
        public const Cpu.Pin PC_3 = (Cpu.Pin) 0x0060C00C2; // GPIO6[2]
        public const Cpu.Pin PC_4 = (Cpu.Pin) 0x0061000C3; // GPIO6[3]
        public const Cpu.Pin PC_5 = (Cpu.Pin) 0x0061400C4; // GPIO6[4]
        public const Cpu.Pin PC_6 = (Cpu.Pin) 0x0061800C5; // GPIO6[5]
        public const Cpu.Pin PC_7 = (Cpu.Pin) 0x0061C00C6; // GPIO6[6]
        public const Cpu.Pin PC_8 = (Cpu.Pin) 0x0062000C7; // GPIO6[7]
        public const Cpu.Pin PC_9 = (Cpu.Pin) 0x0062400C8; // GPIO6[8]
        public const Cpu.Pin PC_10 = (Cpu.Pin) 0x0062800C9; // GPIO6[9]
        public const Cpu.Pin PC_11 = (Cpu.Pin) 0x0062C00CA; // GPIO6[10]
        public const Cpu.Pin PC_12 = (Cpu.Pin) 0x0063000CB; // GPIO6[11]
        public const Cpu.Pin PC_13 = (Cpu.Pin) 0x0063400CC; // GPIO6[12]
        public const Cpu.Pin PC_14 = (Cpu.Pin) 0x0063800CD; // GPIO6[13]

        public const Cpu.Pin PD_0 = (Cpu.Pin) 0x0068000CE; // GPIO6[14]
        public const Cpu.Pin PD_1 = (Cpu.Pin) 0x0068400CF; // GPIO6[15]
        public const Cpu.Pin PD_2 = (Cpu.Pin) 0x0068800D0; // GPIO6[16]
        public const Cpu.Pin PD_3 = (Cpu.Pin) 0x0068C00D1; // GPIO6[17]
        public const Cpu.Pin PD_4 = (Cpu.Pin) 0x0069000D2; // GPIO6[18]
        public const Cpu.Pin PD_5 = (Cpu.Pin) 0x0069400D3; // GPIO6[19]
        public const Cpu.Pin PD_6 = (Cpu.Pin) 0x0069800D4; // GPIO6[20]
        public const Cpu.Pin PD_7 = (Cpu.Pin) 0x0069C00D5; // GPIO6[21]
        public const Cpu.Pin PD_8 = (Cpu.Pin) 0x006A000D6; // GPIO6[22]
        public const Cpu.Pin PD_9 = (Cpu.Pin) 0x006A400D7; // GPIO6[23]
        public const Cpu.Pin PD_10 = (Cpu.Pin) 0x006A800D8; // GPIO6[24]
        public const Cpu.Pin PD_11 = (Cpu.Pin) 0x006AC00D9; // GPIO6[25]
        public const Cpu.Pin PD_12 = (Cpu.Pin) 0x006B000DA; // GPIO6[26]
        public const Cpu.Pin PD_13 = (Cpu.Pin) 0x006B400DB; // GPIO6[27]
        public const Cpu.Pin PD_14 = (Cpu.Pin) 0x006B800DC; // GPIO6[28]
        public const Cpu.Pin PD_15 = (Cpu.Pin) 0x006BC00DD; // GPIO6[29]
        public const Cpu.Pin PD_16 = (Cpu.Pin) 0x006C000DE; // GPIO6[30]

        public const Cpu.Pin PE_0 = (Cpu.Pin) 0x0070000E0; // GPIO7[0]
        public const Cpu.Pin PE_1 = (Cpu.Pin) 0x0070400E1; // GPIO7[1]
        public const Cpu.Pin PE_2 = (Cpu.Pin) 0x0070800E2; // GPIO7[2]
        public const Cpu.Pin PE_3 = (Cpu.Pin) 0x0070C00E3; // GPIO7[3]
        public const Cpu.Pin PE_4 = (Cpu.Pin) 0x0071000E4; // GPIO7[4]
        public const Cpu.Pin PE_5 = (Cpu.Pin) 0x0071400E5; // GPIO7[5]
        public const Cpu.Pin PE_6 = (Cpu.Pin) 0x0071800E6; // GPIO7[6]
        public const Cpu.Pin PE_7 = (Cpu.Pin) 0x0071C00E7; // GPIO7[7]
        public const Cpu.Pin PE_8 = (Cpu.Pin) 0x0072000E8; // GPIO7[8]
        public const Cpu.Pin PE_9 = (Cpu.Pin) 0x0072400E9; // GPIO7[9]
        public const Cpu.Pin PE_10 = (Cpu.Pin) 0x0072800EA; // GPIO7[10]
        public const Cpu.Pin PE_11 = (Cpu.Pin) 0x0072C00EB; // GPIO7[11]
        public const Cpu.Pin PE_12 = (Cpu.Pin) 0x0073000EC; // GPIO7[12]
        public const Cpu.Pin PE_13 = (Cpu.Pin) 0x0073400ED; // GPIO7[13]
        public const Cpu.Pin PE_14 = (Cpu.Pin) 0x0073800EE; // GPIO7[14]
        public const Cpu.Pin PE_15 = (Cpu.Pin) 0x0073C00EF; // GPIO7[15]

        public const Cpu.Pin PF_0 = (Cpu.Pin) 0x0078001E0;
        public const Cpu.Pin PF_1 = (Cpu.Pin) 0x0078400F0; // GPIO7[16]
        public const Cpu.Pin PF_2 = (Cpu.Pin) 0x0078800F1; // GPIO7[17]
        public const Cpu.Pin PF_3 = (Cpu.Pin) 0x0078C00F2; // GPIO7[18]
        public const Cpu.Pin PF_4 = (Cpu.Pin) 0x0079001E0;
        public const Cpu.Pin PF_5 = (Cpu.Pin) 0x0079400F3; // GPIO7[19]
        public const Cpu.Pin PF_6 = (Cpu.Pin) 0x0079800F4; // GPIO7[20]
        public const Cpu.Pin PF_7 = (Cpu.Pin) 0x0079C00F5; // GPIO7[21]
        public const Cpu.Pin PF_8 = (Cpu.Pin) 0x007A000F6; // GPIO7[22]
        public const Cpu.Pin PF_9 = (Cpu.Pin) 0x007A400F7; // GPIO7[23]
        public const Cpu.Pin PF_10 = (Cpu.Pin) 0x007A800F8; // GPIO7[24]
        public const Cpu.Pin PF_11 = (Cpu.Pin) 0x007AC00F9; // GPIO7[25]

        // Common pin names
        // User interfaces: LEDs, buttons
        public const Cpu.Pin LED_YELLOW = P6_11;
        public const Cpu.Pin LED_GREEN = P2_5;
        public const Cpu.Pin LED_RED = LED_YELLOW;
        public const Cpu.Pin LED_BLUE = LED_GREEN;

        public const Cpu.Pin LED1 = LED_YELLOW;
        public const Cpu.Pin LED2 = LED_GREEN;
        public const Cpu.Pin LED3 = LED_GREEN;
        public const Cpu.Pin LED4 = LED_GREEN;

        public const Cpu.Pin BTN1 = P2_7;

        // Serial pins
        public const Cpu.Pin UART0_TX = P6_4;
        public const Cpu.Pin UART0_RX = P6_5;
        public const Cpu.Pin UART1_TX = P5_6;
        public const Cpu.Pin UART1_RX = P1_14;
        public const Cpu.Pin UART1_RTS = P5_2;
        public const Cpu.Pin UART1_CTS = P5_4;
        public const Cpu.Pin UART2_TX = P2_10;
        public const Cpu.Pin UART2_RX = P2_11;
        public const Cpu.Pin UART3_TX = P2_3;
        public const Cpu.Pin UART3_RX = P2_4;

        // Analog pins
        public const Cpu.Pin P_ADC0_0 = P4_3;
        public const Cpu.Pin P_ADC0_1 = P4_1;
        //public const Cpu.Pin P_ADC1_0 = SFP_ADC0_0;
        public const Cpu.Pin P_ADC0_4 = P7_4;
        public const Cpu.Pin P_ADC0_3 = P7_5;
        public const Cpu.Pin P_ADC1_6 = P7_7;

        public const Cpu.Pin P_ADC0 = P_ADC0_0;
        public const Cpu.Pin P_ADC1 = P_ADC0_1;
        //public const Cpu.Pin P_ADC2 = P_ADC1_0;
        public const Cpu.Pin P_ADC3 = P_ADC0_4;
        public const Cpu.Pin P_ADC4 = P_ADC0_3;
        public const Cpu.Pin P_ADC5 = P_ADC1_6;

        public const Cpu.Pin P_DAC0 = P4_4;

        // GPIO Pins
        public const Cpu.Pin GPIO0_0 = P0_0;
        public const Cpu.Pin GPIO0_1 = P0_1;
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
