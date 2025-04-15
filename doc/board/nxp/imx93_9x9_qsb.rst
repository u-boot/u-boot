.. SPDX-License-Identifier: GPL-2.0+

imx93_9x9_qsb
=======================

U-Boot for the NXP i.MX93 Quick Start Board

Quick Start
-----------

- Get and Build the ARM Trusted firmware
- Get the DDR firmware
- Get ahab-container.img
- Build U-Boot
- Boot

Get and Build the ARM Trusted firmware
--------------------------------------

Note: srctree is U-Boot source directory
Get ATF from: https://github.com/nxp-imx/imx-atf/
branch: lf_v2.10

.. code-block:: bash

   $ unset LDFLAGS
   $ make PLAT=imx93 bl31
   $ cp build/imx93/release/bl31.bin $(srctree)

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

   $ wget https://www.nxp.com/lgfiles/NMG/MAD/YOCTO/firmware-sentinel-0.11.bin
   $ chmod +x firmware-sentinel-0.11.bin
   $ ./firmware-sentinel-0.11.bin
   $ cp firmware-sentinel-0.11/mx93a1-ahab-container.img $(srctree)

Build U-Boot
------------

.. code-block:: bash

   $ export CROSS_COMPILE=aarch64-poky-linux-
   $ make imx93_9x9_qsb_defconfig or imx93_9x9_qsb_inline_ecc_defconfig
   $ make

- Inline ECC is to enable DDR ECC feature with imx93_9x9_qsb_inline_ecc_defconfig

Burn the flash.bin to MicroSD card offset 32KB:

.. code-block:: bash

   $ dd if=flash.bin of=/dev/sd[x] bs=1024 seek=32 conv=notrunc

Boot
----

Set Boot switch to SD boot
