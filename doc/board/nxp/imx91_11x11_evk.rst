.. SPDX-License-Identifier: GPL-2.0+

imx91_11x11_EVK
=======================

U-Boot for the NXP i.MX91 11x11 EVK

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
   $ make PLAT=imx91 bl31
   $ cp build/imx91/release/bl31.bin $(srctree)

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

   $ wget  https://www.nxp.com/lgfiles/NMG/MAD/YOCTO/firmware-ele-imx-1.3.0-17945fc.bin
   $ chmod +x firmware-ele-imx-1.3.0-17945fc.bin
   $ ./firmware-ele-imx-1.3.0-17945fc.bin
   $ cp firmware-ele-imx-1.3.0-17945fc/mx91a0-ahab-container.img $(srctree)

Build U-Boot
------------

.. code-block:: bash

   $ export CROSS_COMPILE=aarch64-poky-linux-
   $ make imx91_11x11_evk_defconfig or imx91_11x11_evk_inline_ecc_defconfig
   $ make

- Inline ECC is to enable DDR ECC feature with imx91_11x11_evk_inline_ecc_defconfig

Burn the flash.bin to MicroSD card offset 32KB:

.. code-block:: bash

   $ dd if=flash.bin of=/dev/sd[x] bs=1024 seek=32 conv=notrunc

Boot
----

Set Boot switch to SD boot
