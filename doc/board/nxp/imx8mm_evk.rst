.. SPDX-License-Identifier: GPL-2.0+

imx8mm_evk
==========

U-Boot for the NXP i.MX8MM EVK board

Quick Start
-----------

- Build the ARM Trusted firmware binary
- Get ddr firmware
- Build U-Boot
- Boot

Get and Build the ARM Trusted firmware
--------------------------------------

Note: builddir is U-Boot build directory (source directory for in-tree builds)
Get ATF from: https://source.codeaurora.org/external/imx/imx-atf
branch: imx_5.4.47_2.2.0

.. code-block:: bash

   $ make PLAT=imx8mm bl31
   $ cp build/imx8mm/release/bl31.bin $(builddir)

Get the ddr firmware
--------------------

.. code-block:: bash

   $ wget https://www.nxp.com/lgfiles/NMG/MAD/YOCTO/firmware-imx-8.9.bin
   $ chmod +x firmware-imx-8.9.bin
   $ ./firmware-imx-8.9
   $ cp firmware-imx-8.9/firmware/ddr/synopsys/lpddr4*.bin $(builddir)

Build U-Boot
------------

.. code-block:: bash

   $ export CROSS_COMPILE=aarch64-poky-linux-
   $ make imx8mm_evk_defconfig
   $ export ATF_LOAD_ADDR=0x920000
   $ make

Burn the flash.bin to MicroSD card offset 33KB:

.. code-block:: bash

   $sudo dd if=flash.bin of=/dev/sd[x] bs=1024 seek=33 conv=notrunc
   $sudo dd if=u-boot.itb of=/dev/sdc bs=1024 seek=384 conv=sync

Boot
----
Set Boot switch to SD boot
