.. SPDX-License-Identifier: GPL-2.0+

Amlogic SoC Boot Flow
=====================

The Amlogic SoCs have a pre-defined boot sequence in the SoC ROM code. Here are
the possible boot sources of different SoC families supported by U-Boot:

GX* & AXG family
----------------

+----------+--------------------+-------+-------+---------------+---------------+
|          |   1                | 2     | 3     |    4          |     5         |
+==========+====================+=======+=======+===============+===============+
| S905     | POC=0: SPI NOR     | eMMC  | NAND  | SD Card       | USB Device    |
| S905X    |                    |       |       |               |               |
| S905L    |                    |       |       |               |               |
| S905W    |                    |       |       |               |               |
| S912     |                    |       |       |               |               |
+----------+--------------------+-------+-------+---------------+---------------+
| S805X    | POC=0: SPI NOR     | eMMC  | NAND  | USB Device    | -             |
| A113D    |                    |       |       |               |               |
| A113X    |                    |       |       |               |               |
+----------+--------------------+-------+-------+---------------+---------------+

POC pin: `NAND_CLE`

Some boards provide a button to force USB BOOT which disables the eMMC clock signal
to bypass the eMMC stage. Others have removable eMMC modules; removing the eMMC and
SDCard will allow boot from USB.

An exception is the lafrite board (aml-s805x-xx) which has no SDCard slot and boots
from SPI. The only ways to boot the lafrite board from USB are:

 - Erase the first sectors of SPI NOR flash
 - Insert an HDMI boot plug forcing boot over USB

The VIM1 and initial VIM2 boards provide a test point on the eMMC signals to block
the storage from answering and continue to the next boot step.

The USB Device boot uses the first USB interface. On some boards this port is only
available on an USB-A type connector and needs an special Type-A to Type-A cable to
communicate with the BootROM.

G12* & SM1 family
-----------------

+-------+-------+-------+---------------+---------------+---------------+---------------+
| POC0  | POC1  | POC2  | 1             | 2             | 3             | 4             |
+=======+=======+=======+===============+===============+===============+===============+
| 0     | 0     | 0     | USB Device    | SPI NOR       | NAND/eMMC     | SDCard        |
+-------+-------+-------+---------------+---------------+---------------+---------------+
| 0     | 0     | 1     | USB Device    | NAND/eMMC     | SDCard        | -             |
+-------+-------+-------+---------------+---------------+---------------+---------------+
| 0     | 1     | 0     | SPI NOR       | NAND/eMMC     | SDCard        | USB Device    |
+-------+-------+-------+---------------+---------------+---------------+---------------+
| 0     | 1     | 1     | SPI NAND      | NAND/eMMC     | USB Device    | -             |
+-------+-------+-------+---------------+---------------+---------------+---------------+
| 1     | 0     | 0     | USB Device    | SPI NOR       | NAND/eMMC     | SDCard        |
+-------+-------+-------+---------------+---------------+---------------+---------------+
| 1     | 0     | 1     | USB Device    | NAND/eMMC     | SDCard        | -             |
+-------+-------+-------+---------------+---------------+---------------+---------------+
| 1     | 1     | 0     | SPI NOR       | NAND/eMMC     | SDCard        | USB Device    |
+-------+-------+-------+---------------+---------------+---------------+---------------+
| 1     | 1     | 1     | NAND/eMMC     | SDCard        | USB Device    | -             |
+-------+-------+-------+---------------+---------------+---------------+---------------+

The last option (1/1/1) is the normal default seen on production devices.

 * POC0 pin: `BOOT_4` (0 and all other 1 means SPI NAND boot first)
 * POC1 pin: `BOOT_5` (0 and all other 1 means USB Device boot first
 * POC2 pin: `BOOT_6` (0 and all other 1 means SPI NOR boot first)

Most boards provide a button to force USB BOOT which lowers `BOOT_5` to 0. Some boards
provide a test point on the eMMC or SPI NOR clock signals to block the storage from
answering and continue to the next boot step.

The Khadas VIM3/3L boards embed a microcontroller which sets POC signals according
to its configuration or a specific key press sequence to either boot from SPI NOR
or eMMC then SDCard, or boot as an USB Device.

The Odroid N2/N2+ has a hardware switch to select between SPI NOR or eMMC boot.

Boot Modes
----------

 * SDCard

The BootROM fetches the first SDCard sectors in one sequence, then checks the content
of the data. The BootROM expects to find the FIP binary in sector 1, 512 bytes offset
from the start.

 * eMMC

The BootROM fetches the first sectors in one sequence, first on the main partition,
and then on the Boot0 followed by Boot1 HW partitions. After each read, the BootROM
checks the data and looks to the next partition if it fails. The BootROM expects to
find the FIP binary in sector 1, 512 bytes offset from the start.

 * SPI NOR

The BootROM fetches the first SPI NOR sectors in one sequence, then checks the content
of the data. The BootROM expects to find the FIP binary in sector 1, 512 bytes offset
from the start.

 * NAND & SPI NAND

These modes are rarely used in open platforms and no details are available.

 * USB Device

The BootROM sets the USB Gadget interface to serve a custom USB protocol with the
USB ID 1b8e:c003. The Amlogic `update` utility is designed to use this protocol. It
is also implemented in the Amlogic Vendor U-Boot.

The open-source `pyamlboot` utility https://github.com/superna9999/pyamlboot also
implements this protocol and can load U-Boot in memory in order to start the SoC
without any attached storage or to recover from a failed/incorrect image flash.

HDMI Recovery
-------------

The BootROM also briefly reads 8 bytes at address I2C 0x52 offset 0xf8 (248) on the
HDMI DDC bus. If the content is `boot@USB` it will force USB boot mode. If the content
is `boot@SDC` it will force SDCard boot mode.

If USB Device doesn't enumerate or SD Card boot step doesn't work, the BootROM will
continue with the normal boot sequence.

Special boot dongles can be built by connecting a 256bytes EEPROM set to answer on
address 0x52, and program `boot@USB` or `boot@SDC` at offset 0xf8 (248).

Note: If the SoC is booted with USB Device forced at first step, it will keep the boot
order on warm reboot. Only cold reboot (power removed) will reset the boot order.
