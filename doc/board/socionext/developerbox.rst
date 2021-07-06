.. SPDX-License-Identifier: GPL-2.0+

Introduction
============

DeveloperBox is a certified 96boards Enterprise Edition board. The board/SoC has: -

* Socionext SC2A11 24-cores ARM Cortex-A53 on tbe Mini-ATX form factor motherboard
* 4 DIMM slots (4GB DDR4-2400 UDIMM shipped by default)
* 1 4xPCIe Gen2 slot and 2 1xPCIe Gen2 slots
  (1x slots are connected via PCIe bridge chip)
* 4 USB-3.0 ports
* 2 SATA ports
* 1 GbE network port
* 1 USB-UART serial port (micro USB)
* 64MB SPI NOR Flash
* 8GB eMMC Flash Storage
* 96boards LS connector

The DeveloperBox schematic can be found here: -
https://www.96boards.org/documentation/enterprise/developerbox/hardware-docs/mzsc2am_v03_20180115_a.pdf

And the other documents can be found here: -
https://www.96boards.org/documentation/enterprise/developerbox/


Currently, the U-Boot port supports: -

* USB
* eMMC
* SPI-NOR
* SATA
* GbE

The DeveloperBox boots the TF-A and EDK2 as a main bootloader by default.
The DeveloperBox U-Boot port will replace the EDK2 and boot from TF-A as
BL33, but no need to combine with it.

Compile from source
===================

You can build U-Boot without any additinal source code.::

  cd u-boot
  export ARCH=arm64
  export CROSS_COMPILE=aarch64-linux-gnu-
  make SynQuacer_defconfig
  make -j `noproc`

Then, expand the binary to 1MB for preparing flash.::

  cp u-boot.bin SPI_NOR_UBOOT.fd
  truncate -s 1M SPI_NOR_UBOOT.fd

Installation
============

You can install the SNI_NOR_UBOOT.fd via NOR flash writer.

Flashing the U-Boot image on DeveloperBox requires a 96boards UART mezzanine or other mezzanine which can connect to LS-UART0 port.
Connect USB cable from host to the LS-UART0 and set DSW2-7 to ON, and turn the board on again. The flash writer program will be started automatically; don’t forget to turn the DSW2-7 off again after flashing.

*!!CAUTION!! If you failed to write the U-Boot image on wrong address, the board can be bricked. See below page if you need to recover the bricked board. See the following page for more detail*

https://www.96boards.org/documentation/enterprise/developerbox/installation/board-recovery.md.html

When the serial flasher is running correctly is will show the following boot messages shown via LS-UART0::


  /*------------------------------------------*/
  /*  SC2A11 "SynQuacer" series Flash writer  */
  /*                                          */
  /*  Version: cd254ac                        */
  /*  Build: 12/15/17 11:25:45                */
  /*------------------------------------------*/

  Command Input >

Once the flasher tool is running we are ready flash the UEFI image::

  flash rawwrite 200000 100000
  >> Send SPI_NOR_UBOOT.fd via XMODEM (Control-A S in minicom) <<

*!!NOTE!! The flasher command parameter is different from the command for board recovery. U-Boot uses the offset 200000 (2-five-0, 2M in hex) and the size 100000 (1-five-0, 1M in hex).*

After transferring the SPI_NOR_UBOOT.fd, turn off the DSW2-7 and reset the board.

