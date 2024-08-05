.. SPDX-License-Identifier: GPL-2.0+

imx8ulp_evk
=======================

U-Boot for the NXP i.MX 8ULP EVK board

Quick Start
-----------

- Get and Build the ARM Trusted firmware
- Get the uPower firmware
- Get the M33 firmware
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
   $ make PLAT=imx8ulp bl31
   $ cp build/imx8ulp/release/bl31.bin $(srctree)

Get the uPower firmware
-----------------------

.. code-block:: bash

   $ wget https://www.nxp.com/lgfiles/NMG/MAD/YOCTO/firmware-upower-1.3.1.bin
   $ chmod +x firmware-upower-1.3.1.bin
   $ ./firmware-upower-1.3.1.bin
   $ cp firmware-upower-1.3.1/upower_a1.bin $(srctree)/upower.bin

Get the M33 firmware
--------------------

.. code-block:: bash

   $ wget http://www.nxp.com/lgfiles/NMG/MAD/YOCTO/imx8ulp-m33-demo-2.14.1.bin
   $ chmod +x imx8ulp-m33-demo-2.14.1.bin
   $ ./imx8ulp-m33-demo-2.14.1.bin
   $ cp imx8ulp-m33-demo-2.14.1/imx8ulp_m33_TCM_power_mode_switch.bin $(srctree)/m33_image.bin

Get ahab-container.img
---------------------------------------

.. code-block:: bash

   $ wget https://www.nxp.com/lgfiles/NMG/MAD/YOCTO/firmware-ele-imx-0.1.2-4ed450a.bin
   $ chmod +x firmware-ele-imx-0.1.2-4ed450a.bin
   $ ./firmware-ele-imx-0.1.2-4ed450a.bin
   $ cp firmware-ele-imx-0.1.2-4ed450a/mx8ulpa2-ahab-container.img $(srctree)

Build U-Boot
------------

.. code-block:: bash

   $ export CROSS_COMPILE=aarch64-poky-linux-
   $ make imx8ulp_evk_defconfig
   $ make

Burn the flash.bin to MicroSD card offset 32KB:

.. code-block:: bash

   $ dd if=flash.bin of=/dev/sd[x] bs=1024 seek=32 conv=notrunc

Boot
----

Set Boot switch to SD boot
