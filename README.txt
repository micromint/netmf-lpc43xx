NetMF port to NXP LPC43xx
=========================
Updated: 08/26/13

The NXP LPC43xx microcontrollers are the first to include multiple Cortex-M
cores in a single microcontroller package. This port allows NetMF developers
to take advantage of the LPC43xx in their application using APIs that they
are familiar with. Some of the key features of the LPC43xx include:

* Dual core ARM Cortex-M4/M0 both capable of up to 204 MHz
* Up to 264 KB SRAM, 1 MB internal flash
* Two High-speed USB 2.0 interfaces
* Ethernet MAC
* LCD interface
* Quad-SPI Flash Interface (SPIFI)
* State Configurable Timer (SCT)
* Serial GPIO (SGPIO)
* Up to 164 GPIO

The NXP LPC18xx is a single core Cortex-M3 implementation that is compatible
with the LPC43XX for cost-sensitive applications not requiring multiple cores.

NetMF port to the LPC43XX - Micromint USA <support@micromint.com>

Compatibility
-------------
* This port has been tested with the following boards:
    Board                    MCU        RAM/Flash
    Micromint Bambino 200    LPC4330    264K SRAM/4 MB SPIFI flash
	
* The USB driver currently uses a CDC class while we correct an issue with the
  custom class used for WinUSB. The board will enumerate as a COM port. Drivers
  are in the Micromint Bambino Wiki (wiki.micromint.com).

* This port has been developed and tested with the MDK 4.71. We will be
  supporting the GCC CodeRed toolchain in early September.

* Ethernet, USB and microSD filesystem drivers will be available when the
  Bambino 200E is released.

* This port has been developed and tested with the MDK 4.71. We will be
  supporting the GCC CodeRed toolchain in early September.

Notes
-----
* On the LPC43xx the hardware pin name and the GPIO pin name are not the same,
  requiring different offsets for the SCU and GPIO registers. To simplify logic
  the pin identifier encodes the offsets. Use CPU_PIN_Px_yy for CPU pins and
  GPIO_PIN_Px_yy for GPIO pins. See the CPU.cs in the LPC43XX device code
  for an example enumeration.
