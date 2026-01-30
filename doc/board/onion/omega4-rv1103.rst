.. SPDX-License-Identifier: GPL-2.0+

Onion Omega4 RV1103 board
=========================

U-Boot for the Onion Omega4 RV1103 board

Quick Start
-----------

- Get the DDR initialization binary
- Build U-Boot
- Flash U-Boot into the SPI NAND

Get the DDR initialization binary
---------------------------------

.. code-block:: bash

   $ git clone https://github.com/rockchip-linux/rkbin.git

The RV1103 DDR initialization is located at rkbin/bin/rv11/rv1103b_ddr_924MHz_v1.05.bin

Build U-Boot
------------

.. code-block:: bash

   $ export CROSS_COMPILE=arm-linux-gnueabihf-
   $ export ROCKCHIP_TPL=<path-to-rkbin>/bin/rv11/rv1103b_ddr_924MHz_v1.05.bin
   $ make omega4-rv1103_defconfig
   $ make

The idbloader-spi.img and u-boot.img are the binaries that need to be flashed
into the SPI NAND.

Flash U-Boot into the SPI NAND
------------------------------

Connect the USB OTG and UART console cables from the Omega4 board to
the host PC.

Press the BOOT button while applying power to the board.

The string "RKUART" should appear on the console (115200,8N1).

Install the rkdeveloptool from https://github.com/rockchip-linux/rkdeveloptool
by following the instruction in the README file.

.. code-block:: bash

   $ sudo ./rkdeveloptool db download.bin
   $ sudo ./rkdeveloptool wl 0x200 idbloader.img
   $ sudo ./rkdeveloptool wl 0xa00 u-boot.img

Power cycle the board and U-Boot output is seen on the console.
