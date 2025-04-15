.. SPDX-License-Identifier: GPL-2.0+

U-Boot for Libre Computer AML-A311D-CC 'Alta' (A311D)
=====================================================

AML-A311D-CC is a Single Board Computer manufactured by Libre Computer Technology with
the following specifications:

 - Amlogic A311D Arm Cortex-A53 dual-core + Cortex-A73 quad-core SoC
 - 2 or 4GB LPDDR4 SDRAM
 - Gigabit Ethernet
 - HDMI 2.1 display
 - 40-pin GPIO header
 - 4 x USB 3.0 Host, 1 x USB 2.0 Type-C
 - eMMC 5.x SM Interface for Libre Computer Modules
 - microSD
 - Infrared receiver

Schematics are available on the manufacturer website.

U-Boot Compilation
------------------

.. code-block:: bash

    $ export CROSS_COMPILE=aarch64-none-elf-
    $ make aml-a311d-cc_defconfig
    $ make

U-Boot Signing with Pre-Built FIP repo
--------------------------------------

.. code-block:: bash

    $ git clone https://github.com/LibreELEC/amlogic-boot-fip --depth=1
    $ cd amlogic-boot-fip
    $ mkdir my-output-dir
    $ ./build-fip.sh aml-a311d-cc /path/to/u-boot/u-boot.bin my-output-dir

Then write U-Boot to SD or eMMC with:

.. code-block:: bash

    $ DEV=/dev/boot_device
    $ dd if=fip/u-boot.bin.sd.bin of=$DEV conv=fsync,notrunc bs=512 skip=1 seek=1
    $ dd if=fip/u-boot.bin.sd.bin of=$DEV conv=fsync,notrunc bs=1 count=440
