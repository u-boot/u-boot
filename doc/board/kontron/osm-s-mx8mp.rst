.. SPDX-License-Identifier: GPL-2.0+

Kontron Electronics i.MX8MP SoMs and Boards
===========================================

The OSM-S i.MX8MP by Kontron Electronics GmbH is a SoM module with an
i.MX8M-Plus SoC, up to 8 GB LPDDR4 RAM, eMMC, PMIC, RTC.

The matching evaluation boards (Board-Line, BL) have two Ethernet ports,
USB 2.0, HDMI/LVDS, SD card, CAN, RS485, RS232 and much more.

The OSM-S i.MX8MP is compliant to the Open Standard Module (OSM) 1.1
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

There are two sources for the TF-A. Mainline and NXP. Get the one you prefer
(support and features might differ).

.. note::

   If you are using GCC 12 and you get compiler/linker errors, try to add the
   following arguments to your make command as workaround:
   ``CFLAGS="-Wno-array-bounds" LDFLAGS="--no-warn-rwx-segments"``

**NXP's imx-atf**

1. Get TF-A from: https://github.com/nxp-imx/imx-atf, branch: lf_v2.6
2. Build

  .. code-block:: bash

     $ make PLAT=imx8mp CROSS_COMPILE=aarch64-linux-gnu- IMX_BOOT_UART_BASE="0x30880000" bl31
     $ cp build/imx8mp/release/bl31.bin $(builddir)

.. note::

    *builddir* is U-Boot's build directory (source directory for in-tree builds)

**Mainline TF-A**

1. Get TF-A from: https://git.trustedfirmware.org/TF-A/trusted-firmware-a.git/, tag: v2.4
2. Build

  .. code-block:: bash

     $ make PLAT=imx8mp CROSS_COMPILE=aarch64-linux-gnu- IMX_BOOT_UART_BASE="0x30880000" bl31
     $ cp build/imx8mp/release/bl31.bin $(builddir)

Get the DDR firmware
^^^^^^^^^^^^^^^^^^^^

.. code-block:: bash

   $ wget https://www.nxp.com/lgfiles/NMG/MAD/YOCTO/firmware-imx-8.18.bin
   $ chmod +x firmware-imx-8.18.bin
   $ ./firmware-imx-8.18.bin
   $ cp firmware-imx-8.18/firmware/ddr/synopsys/lpddr4_pmu_train_1d_imem_202006.bin $(builddir)
   $ cp firmware-imx-8.18/firmware/ddr/synopsys/lpddr4_pmu_train_1d_dmem_202006.bin $(builddir)
   $ cp firmware-imx-8.18/firmware/ddr/synopsys/lpddr4_pmu_train_2d_imem_202006.bin $(builddir)
   $ cp firmware-imx-8.18/firmware/ddr/synopsys/lpddr4_pmu_train_2d_dmem_202006.bin $(builddir)

Build U-Boot
^^^^^^^^^^^^

.. code-block:: bash

   $ make kontron-osm-s-mx8mp_defconfig
   $ make CROSS_COMPILE=aarch64-linux-gnu-

Copy the flash.bin to SD card at an offset of 32 KiB:

.. code-block:: bash

   $ dd if=flash.bin of=/dev/sd[x] bs=1K seek=32 conv=notrunc

Boot
^^^^

Put the SD card in the slot on the board and apply power. Check the serial
console for output.

Further Information
-------------------

The bootloader configuration is setup to be used with kernel FIT images. Legacy
images might not be working out of the box.

Please see https://docs.kontron-electronics.de for further vendor documentation.
