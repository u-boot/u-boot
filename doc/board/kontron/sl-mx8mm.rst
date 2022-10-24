.. SPDX-License-Identifier: GPL-2.0+

Kontron Electronics i.MX8MM SoMs and Boards
===========================================

The SL i.MX8MM and OSM-S i.MX8MM by Kontron Electronics GmbH are SoM modules
with an i.MX8M-Mini SoC, 1/2/4 GB LPDDR4 RAM, SPI NOR, eMMC and PMIC.

The matching evaluation boards (Board-Line, BL) have two Ethernet ports,
USB 2.0, HDMI/LVDS, SD card, CAN, RS485, RS232 and much more.

The OSM-S i.MX8MM is compliant to the Open Standard Module (OSM) 1.0
specification, size S (https://sget.org/standards/osm).

Quick Start
-----------

- Get and Build the Trusted Firmware-A (TF-A)
- Get the DDR firmware
- Build U-Boot
- Boot

Get and Build the Trusted Firmware-A (TF-A)
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

Note: builddir is U-Boot build directory (source directory for in-tree builds)

There are two sources for the TF-A. Mainline and NXP. Get the one you prefer
(support and features might differ).

**NXP's imx-atf**

1. Get TF-A from: https://source.codeaurora.org/external/imx/imx-atf, branch: imx_5.4.70_2.3.0
2. Apply the patch to select the correct UART for the console, otherwise the TF-A will lock up during boot.
3. Build

  .. code-block:: bash

     $ make PLAT=imx8mm bl31
     $ cp build/imx8mm/release/bl31.bin $(builddir)

**Mainline TF-A**

1. Get TF-A from: https://git.trustedfirmware.org/TF-A/trusted-firmware-a.git/, tag: v2.4
2. Build

  .. code-block:: bash

     $ make PLAT=imx8mm CROSS_COMPILE=aarch64-linux-gnu- IMX_BOOT_UART_BASE="0x30880000" bl31
     $ cp build/imx8mm/release/bl31.bin $(builddir)

Get the DDR firmware
^^^^^^^^^^^^^^^^^^^^

.. code-block:: bash

   $ wget https://www.nxp.com/lgfiles/NMG/MAD/YOCTO/firmware-imx-8.9.bin
   $ chmod +x firmware-imx-8.9.bin
   $ ./firmware-imx-8.9.bin
   $ cp firmware-imx-8.9/firmware/ddr/synopsys/lpddr4*.bin $(builddir)

Build U-Boot
^^^^^^^^^^^^

.. code-block:: bash

   $ make kontron-sl-mx8mm_defconfig
   $ make

Burn the flash.bin to SD card at an offset of 33 KiB:

.. code-block:: bash

   $ dd if=flash.bin of=/dev/sd[x] bs=1K seek=33 conv=notrunc

Boot
^^^^

Put the SD card in the slot on the board and apply power.

Further Information
-------------------

The bootloader configuration is setup to be used with kernel FIT images. Legacy
images might not be working out of the box.

Please see https://docs.kontron-electronics.de for further vendor documentation.
