.. SPDX-License-Identifier: GPL-2.0+

Kontron Electronics i.MX93 SoMs and Boards
===========================================

The OSM-S i.MX93 by Kontron Electronics GmbH is a SoM module with an
i.MX93 SoC, 2 GB LPDDR4 RAM, eMMC, PMIC, RTC.

The matching evaluation boards (Board-Line, BL) have two Ethernet ports,
USB 2.0, LVDS, SD card, CAN, RS485, RS232 and much more.

The OSM-S i.MX93 is compliant to the Open Standard Module (OSM) 1.1
specification, size S (https://sget.org/standards/osm).

Quick Start
-----------

- Get and Build the Trusted Firmware-A (TF-A)
- Get the DDR firmware
- Build U-Boot
- Boot

.. note::

   To build on a x86-64 host machine, you need a GNU cross toolchain for the
   target architecture (aarch64). Check your distros package manager or
   download and install the necessary tools (``aarch64-linux-gnu-*``) manually.

Get and Build the Trusted Firmware-A (TF-A)
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

.. note::

   If you are using GCC 12 and you get compiler/linker errors, try to add the
   following arguments to your make command as workaround:
   ``CFLAGS="-Wno-array-bounds" LDFLAGS="--no-warn-rwx-segments"``

1. Get TF-A from: https://github.com/nxp-imx/imx-atf, branch: lf_v2.12
2. Build

  .. code-block:: bash

     $ make PLAT=imx93 CROSS_COMPILE=aarch64-linux-gnu- IMX_BOOT_UART_BASE="0x30880000" bl31
     $ cp build/imx93/release/bl31.bin $(builddir)

.. note::

    *builddir* is U-Boot's build directory (source directory for in-tree builds)

Get the DDR firmware
^^^^^^^^^^^^^^^^^^^^

.. code-block:: bash

   $ wget https://www.nxp.com/lgfiles/NMG/MAD/YOCTO/firmware-imx-8.28-994fa14.bin
   $ wget https://www.nxp.com/lgfiles/NMG/MAD/YOCTO/firmware-ele-imx-2.0.2-89161a8.bin
   $ chmod +x firmware-imx-8.28-994fa14.bin firmware-ele-imx-2.0.2-89161a8.bin
   $ ./firmware-imx-8.28-994fa14.bin
   $ ./firmware-ele-imx-2.0.2-89161a8.bin
   $ cp firmware-imx-8.28-994fa14/firmware/ddr/synopsys/lpddr4_dmem_1d_v202201.bin $(builddir)
   $ cp firmware-imx-8.28-994fa14/firmware/ddr/synopsys/lpddr4_dmem_2d_v202201.bin $(builddir)
   $ cp firmware-imx-8.28-994fa14/firmware/ddr/synopsys/lpddr4_imem_1d_v202201.bin $(builddir)
   $ cp firmware-imx-8.28-994fa14/firmware/ddr/synopsys/lpddr4_imem_2d_v202201.bin $(builddir)
   $ cp firmware-ele-imx-2.0.2-89161a8/mx93a1-ahab-container.img $(builddir)

Build U-Boot
^^^^^^^^^^^^

.. code-block:: bash

   $ make kontron-osm-s-mx93_defconfig
   $ make CROSS_COMPILE=aarch64-linux-gnu-

Copy the flash.bin to SD card at an offset of 32 KiB:

.. code-block:: bash

   $ dd if=flash.bin of=/dev/sd[x] bs=1K seek=32 conv=fsync

Boot
^^^^

Put the SD card in the slot on the board and apply power. Alternatively connect
the USB Type-C Connector to your host machine, power up the board and load the
bootloader via uuu-Tool (`uuu flash.bin`).

Check the serial console for output.

Further Information
-------------------

The bootloader configuration is setup to be used with kernel FIT images. Legacy
images might not be working out of the box.

Please see https://docs.kontron-electronics.de for further vendor documentation.
