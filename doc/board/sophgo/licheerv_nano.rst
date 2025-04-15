.. SPDX-License-Identifier: GPL-2.0+

LicheeRV Nano
=============

SG2002 RISC-V SoC
-----------------
The SG2002 is a high-performance, low-power 64-bit RISC-V/ARM SoC from Sophgo.

Mainline support
----------------
The support for following drivers are already enabled:
1. ns16550 UART Driver.
2. Synopsys Designware MSHC Driver

Building
~~~~~~~~
1. Add the RISC-V toolchain to your PATH.
2. Setup ARCH & cross compilation environment variable:

.. code-block:: console

   export CROSS_COMPILE=<riscv64 toolchain prefix>
   cd <U-Boot-dir>
   make sipeed_licheerv_nano_defconfig
   make

This will generate u-boot.bin

Booting
~~~~~~~
Currently, we rely on vendor FSBL (First Stage Boot Loader) to initialize the
clock and load the u-boot image, then bootup from it.

To run u-boot.bin on top of FSBL, follow these steps:

1. Use mainline OpenSBI with a newer version than 1.5 to generate fw_dynamic.

2. Generate a compatible u-boot.bin using U-Boot with the LicheeRV Nano default
   configuration.

3. Use the vendor-provided tool [1] to create a unified fip.bin file containing
   FSBL, OpenSBI, and U-Boot.
   Note that you will have to use the file cv181x.bin as the FSBL.

2. Place the generated fip.bin file into the FAT partition of the SD card.

3. Insert the SD card into the board and power it on.

The board will automatically execute the FSBL from the fip.bin file.
Subsequently, it will transition to OpenSBI, and finally, OpenSBI will invoke
U-Boot.

[1]: https://github.com/sophgo/fiptool


Sample boot log from LicheeRV Nano board
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
.. code-block:: none

   U-Boot 2024.10 (Oct 24 2024 - 15:00:20 +0200)licheerv_nano

   DRAM:  256 MiB
   Core:  19 devices, 11 uclasses, devicetree: separate
   MMC:   mmc@4310000: 0
   Loading Environment from nowhere... OK
   In:    serial@4140000
   Out:   serial@4140000
   Err:   serial@4140000
   Net:   No ethernet found.
   Hit any key to stop autoboot:  0
   licheerv_nano#
