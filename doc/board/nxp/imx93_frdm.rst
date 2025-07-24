.. SPDX-License-Identifier: GPL-2.0+

imx93_frdm
==========

U-Boot for the NXP i.MX93 FRDM board

Quick Start
-----------

- Get and Build the ARM Trusted firmware
- Get the DDR firmware
- Get ahab-container.img
- Build U-Boot
- Boot from the SD card

Get and Build the ARM Trusted firmware
--------------------------------------

Note: srctree is U-Boot source directory
Get ATF from: https://github.com/nxp-imx/imx-atf/
branch: lf_v2.8

.. code-block:: bash

   $ unset LDFLAGS
   $ make PLAT=imx93 bl31
   $ cp build/imx93/release/bl31.bin $(srctree)

Get the DDR firmware
--------------------

.. code-block:: bash

   $ wget https://www.nxp.com/lgfiles/NMG/MAD/YOCTO/firmware-imx-8.21.bin
   $ chmod +x firmware-imx-8.21.bin
   $ ./firmware-imx-8.21.bin
   $ cp firmware-imx-8.21/firmware/ddr/synopsys/lpddr4*.bin $(srctree)

Get ahab-container.img
----------------------

.. code-block:: bash

   $ wget https://www.nxp.com/lgfiles/NMG/MAD/YOCTO/firmware-sentinel-0.11.bin
   $ chmod +x firmware-sentinel-0.11.bin
   $ ./firmware-sentinel-0.11.bin
   $ cp firmware-sentinel-0.11/mx93a1-ahab-container.img $(srctree)

Build U-Boot
------------

.. code-block:: bash

   $ export CROSS_COMPILE=aarch64-poky-linux-
   $ make imx93_frdm_defconfig
   $ make

Copy the flash.bin binary to the MicroSD card at offset 32KB:

.. code-block:: bash

   $ dd if=flash.bin of=/dev/sd[x] bs=1k seek=32; sync

Boot from the SD card
---------------------

- Configure SW1 boot switches to SD boot mode:
  0011 SW1[3:0] - ("USDHC2 4-bit SD3.0" Boot Mode)
- Insert the SD card in the SD slot (P13) of the board.
- Connect a USB Type-C cable into the P16 Debug USB Port and connect
  using a terminal emulator at 115200 bps, 8n1. The console will show up
  at /dev/ttyACM0.
- Power on the board by connecting a USB Type-C cable into the P1
  Power USB Port.
