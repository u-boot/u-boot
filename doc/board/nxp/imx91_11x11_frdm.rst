.. SPDX-License-Identifier: GPL-2.0+

imx91_frdm
=======================

U-Boot for the NXP i.MX91 11x11 FRDM Board

Quick Start
-----------

- Get and Build the ARM Trusted firmware
- Get the DDR firmware
- Get ahab-container.img
- Build U-Boot
- Boot from the SD card
- Boot using USB serial download (uuu)

Get and Build the ARM Trusted firmware
--------------------------------------

Note: srctree is U-Boot source directory
Get ATF from: https://github.com/nxp-imx/imx-atf/
branch: lf_v2.10

.. code-block:: bash

   $ unset LDFLAGS
   $ make PLAT=imx91 bl31
   $ cp build/imx91/release/bl31.bin $(srctree)

Get the DDR firmware
--------------------

.. code-block:: bash

   $ wget https://www.nxp.com/lgfiles/NMG/MAD/YOCTO/firmware-imx-8.21.bin
   $ chmod +x firmware-imx-8.21.bin
   $ ./firmware-imx-8.21.bin
   $ cp firmware-imx-8.21/firmware/ddr/synopsys/lpddr4*.bin $(srctree)

Get ahab-container.img
---------------------------------------

.. code-block:: bash

   $ wget  https://www.nxp.com/lgfiles/NMG/MAD/YOCTO/firmware-ele-imx-1.3.0-17945fc.bin
   $ chmod +x firmware-ele-imx-1.3.0-17945fc.bin
   $ ./firmware-ele-imx-1.3.0-17945fc.bin
   $ cp firmware-ele-imx-1.3.0-17945fc/mx91a0-ahab-container.img $(srctree)

Build U-Boot
------------

.. code-block:: bash

   $ export CROSS_COMPILE=aarch64-poky-linux-
   $ make imx91_11x11_frdm_defconfig or imx91_11x11_frdm_inline_ecc_defconfig
   $ make

- Inline ECC is to enable DDR ECC feature with imx91_11x11_frdm_inline_ecc_defconfig
  Enable ECC will reduce DDR size by 1/8. For 1GB DRAM, available size will be 896MB.

Burn the flash.bin to MicroSD card offset 32KB:

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

Boot using USB serial download (uuu)
------------------------------------

- Configure SW1 boot switches to serial download boot mode:
  0001 SW1[3:0] - ("Serial downloader (USB)" Boot Mode)
- Plug USB Type-C cable into the P2 device port.
- Connect a USB Type-C cable into the P16 Debug USB Port and connect
  using a terminal emulator at 115200 bps, 8n1. The console will show up
  at /dev/ttyACM0.
- Power on the board by connecting a USB Type-C cable into the P1
  Power USB Port.
- Use NXP Universal Update Utility `NXP Universal Update Utility`_ to boot or
  flash the device. E.g. following command can be used to flash an image onto
  the eMMC storage:

.. code-block:: bash

   $ uuu -V -b emmc_all <image file>

.. _`NXP Universal Update Utility`: https://github.com/nxp-imx/mfgtools