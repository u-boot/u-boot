.. SPDX-License-Identifier: GPL-2.0+

imx93w_evk
==========

U-Boot for the NXP i.MX93W EVK board.

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
branch: lf_v2.14

.. code-block:: bash

   $ git clone -b lf_v2.14 https://github.com/nxp-imx/imx-atf.git
   $ cd imx-atf
   $ export CROSS_COMPILE=aarch64-poky-linux-
   $ unset LDFLAGS
   $ unset AS
   $ make PLAT=imx93 bl31
   $ cp build/imx93/release/bl31.bin $(srctree)

Get the DDR firmware
--------------------

.. code-block:: bash

   $ wget https://www.nxp.com/lgfiles/NMG/MAD/YOCTO/firmware-imx-8.32-1991416.bin
   $ chmod +x firmware-imx-8.32-1991416.bin
   $ ./firmware-imx-8.32-1991416.bin
   $ cp firmware-imx-8.32-1991416/firmware/ddr/synopsys/lpddr4*.bin $(srctree)

Get ahab-container.img
----------------------

.. code-block:: bash

   $ wget https://www.nxp.com/lgfiles/NMG/MAD/YOCTO/firmware-ele-imx-2.0.6-c0b284c.bin
   $ chmod +x firmware-ele-imx-2.0.6-c0b284c.bin
   $ ./firmware-ele-imx-2.0.6-c0b284c.bin
   $ cp firmware-ele-imx-2.0.6-c0b284c/mx93a1-ahab-container.img $(srctree)

Build U-Boot
------------

.. code-block:: bash

   $ export CROSS_COMPILE=aarch64-poky-linux-
   $ make imx93w_evk_defconfig
   $ make

Burn the flash.bin to MicroSD card offset 32KB:

.. code-block:: bash

   $ dd if=flash.bin of=/dev/sd[x] bs=1024 seek=32 conv=notrunc

Boot
----

Set Boot switch to SD boot.
