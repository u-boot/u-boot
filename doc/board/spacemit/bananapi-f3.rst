.. SPDX-License-Identifier: GPL-2.0-or-later

Banana Pi BPI-F3
================

Building
~~~~~~~~
1. Install the SpacemiT riscv cross compile toolchain_, or skip it if riscv toolchain is installed.

.. _toolchain: https://archive.spacemit.com/toolchain/

2. Setup cross compilation environment variable:

.. code-block:: console

   export CROSS_COMPILE=<riscv64 toolchain prefix, e.g /opt/spacemit/bin/riscv64-unknown-linux-gnu->

3. Before building U-Boot, OpenSBI should be built first. OpenSBI can be
built for SpacemiT K1 SoC as below:

.. code-block:: console

   git clone https://github.com/cyyself/opensbi -b k1-opensbi
   cd opensbi
   make PLATFORM=generic

4. Then build U-Boot as following:

.. code-block:: console

   cd <U-Boot-dir>
   make bananapi-f3_defconfig
   make OPENSBI=<OpenSBI-dir>/build/platform/generic/firmware/fw_dynamic.bin

This will generate u-boot.itb

Burning
~~~~~~~
Actually, we can replace the uboot partition of Bianbu Linux which is the bsp_ to validate this patch,
use `balena etcher` to burn the bianbu-minimal.img to the sd card,
and replace the /dev/sdx4 where places the uboot_ with the `u-boot.itb` generated from this patch.
Or use fastboot:
Collect FSBL.bin, u-boot.itb, partition_2M.json, bootinfo_spinor.bin
u-boot-env-default.bin, fw_dynamic.itb from vendor SDK

.. code-block:: console

   fastboot stage FSBL.bin
   fastboot continue
   fastboot stage u-boot.itb-vendor # the itb from vendor uboot
   fastboot continue

   fastboot flash mtd partition_2M.json
   fastboot flash bootinfo bootinfo_spinor.bin
   fastboot flash fsbl FSBL.bin
   fastboot flash env u-boot-env-default.bin
   fastboot flash opensbi fw_dynamic.itb

   fastboot flash uboot u-boot.itb-mainline # the itb from mainline uboot

.. _bsp: https://archive.spacemit.com/image/k1/version/bianbu/v2.0/
.. _uboot: https://bianbu-linux.spacemit.com/en/device/boot#21-firmware-layout

Booting
~~~~~~~
Sample boot log from Banana Pi BPI-F3 board
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
.. code-block:: none

   try sd...
   bm:3
   j...

   U-Boot SPL 2022.10spacemit-dirty (Oct 21 2024 - 09:01:13 +0000)
   [   0.279] DDR type LPDDR4X
   [   0.292] lpddr4_silicon_init consume 13ms
   [   0.293] Change DDR data rate to 2400MT/s
   [   0.430] ## Checking hash(es) for config conf-1 ... OK
   [   0.432] ## Checking hash(es) for Image opensbi ... OK
   [   0.437] ## Checking hash(es) for Image uboot ... OK
   [   0.443] ## Checking hash(es) for Image fdt-1 ... OK
   [   0.488] ## Checking hash(es) for config config_1 ... OK
   [   0.490] ## Checking hash(es) for Image opensbi ... crc32+ OK


   U-Boot 2024.10-rc4-00462-g5b138cfcc587-dirty (Nov 28 2024 - 14:56:49 +0800)

   DRAM:  4 GiB
   Core:  19 devices, 8 uclasses, devicetree: separate
   Loading Environment from nowhere... OK
   In:    serial@d4017000
   Out:   serial@d4017000
   Err:   serial@d4017000
   Net:   No ethernet found.
   => cpu list
   0: cpu@0      spacemit,x60
   1: cpu@1      spacemit,x60
   2: cpu@2      spacemit,x60
   3: cpu@3      spacemit,x60
   4: cpu@4      spacemit,x60
   5: cpu@5      spacemit,x60
   6: cpu@6      spacemit,x60
   7: cpu@7      spacemit,x60
   => test
   =>

