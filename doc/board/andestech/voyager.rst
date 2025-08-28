.. SPDX-License-Identifier: GPL-2.0+

Voyager
=======

Qilai RISC-V SoC
----------------
The QiLai SoC chip is Andes' first RISC-V SoC. It includes high performance
quad-core Andes AX45MP cluster and one NX27V vector processor.

The Voyager development platform is based on Qilai and capable of running Linux.

Mainline support
----------------

The support for following drivers are already enabled:

1. UART driver
2. MMC driver
3. SPI driver

Building
~~~~~~~~

How to build U-Boot SPL
~~~~~~~~~~~~~~~~~~~~~~~
Before building U-Boot SPL, OpenSBI must be build first.
OpenSBI can be cloned and build for Voyager as below:

1. Get the RISC-V toolchain.
2. Setup cross compilation environment variable.

.. code-block:: none

    git clone https://github.com/riscv-software-src/opensbi.git
    cd opensbi
    make PLATFORM=generic

Copy OpenSBI FW_DYNAMIC image (build/platform/generic/firmware/fw_dynamic.bin)
into U-Boot root directory, then

.. code-block:: console

   export CROSS_COMPILE=riscv64-linux-gnu-
   cd <U-Boot-dir>
   cp fw_dynamic.bin .
   make voyager_spl_defconfig
   make

Booting
~~~~~~~

Currently, we rely on vendor ROM code to initialize the DDR
and load the u-boot image, then boot from it.

Sample boot log from Voyager board
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

.. code-block:: none

    U-Boot SPL 2025.10-rc1-00130-ga28bcbba4778-dirty (Aug 06 2025 - 17:46:10 +0800)
    Trying to boot from RAM

    U-Boot 2025.10-rc1-00130-ga28bcbba4778-dirty (Aug 06 2025 - 17:46:10 +0800)

    CPU:   riscv
    Model: andestech,ax45
    DRAM:  16 GiB
    Core:  25 devices, 14 uclasses, devicetree: board
    MMC:   mmc@30c00000: 0
    Loading Environment from SPIFlash... SF: Detected mx25u1635e with page size 256 Bytes, erase size 4 KiB, total 2 MiB
    *** Warning - bad CRC, using default environment

    In:    serial@30300000
    Out:   serial@30300000
    Err:   serial@30300000
    Net:   No ethernet found.
    Hit any key to stop autoboot: 0
    No ethernet found.
    No ethernet found.
    RISC-V #
