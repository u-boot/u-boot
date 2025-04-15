.. SPDX-License-Identifier: GPL-2.0+

phyGATE-Tauri-L-i.MX 8M Mini
============================

The phyGATE-Tauri-L-i.MX 8M Mini with 2GB of main memory is supported.

Quick Start
-----------

- Build the ARM Trusted firmware binary
- Build the OP-TEE binary (optional)
- Get ddr firmware
- Build U-Boot
- Boot

Build the ARM Trusted firmware binary
-------------------------------------

.. code-block:: bash

   $ git clone https://git.trustedfirmware.org/TF-A/trusted-firmware-a.git
   $ cd trusted-firmware-a
   $ export CROSS_COMPILE=aarch64-linux-gnu-
   $ export IMX_BOOT_UART_BASE=0x30880000
   $ # with optee
   $ make PLAT=imx8mm BL32_BASE=0x56000000 SPD=opteed bl31
   $ # without optee
   $ make PLAT=imx8mm bl31

.. include:: imx8mm-optee-build.rsti

Get the ddr firmware
--------------------

.. code-block:: bash

   $ wget https://www.nxp.com/lgfiles/NMG/MAD/YOCTO/firmware-imx-8.23.bin
   $ chmod +x firmware-imx-8.23.bin
   $ ./firmware-imx-8.23.bin

Build U-Boot for SD card
------------------------

Copy binaries
^^^^^^^^^^^^^

.. code-block:: bash

   $ cp <TF-A dir>/build/imx8mm/release/bl31.bin .
   $ cp <OP-TEE dir>/out/arm/core/tee-raw.bin tee.bin
   $ cp firmware-imx-8.23/firmware/ddr/synopsys/lpddr4*.bin .

Build U-Boot
^^^^^^^^^^^^

.. code-block:: bash

   $ make imx8mm-phygate-tauri-l_defconfig
   $ make flash.bin

Flash SD card
^^^^^^^^^^^^^

.. code-block:: bash

   $ sudo dd if=flash.bin of=/dev/sd[x] bs=1024 seek=33 conv=sync
