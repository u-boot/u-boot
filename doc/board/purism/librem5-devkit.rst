.. SPDX-License-Identifier: GPL-2.0+

Librem5
=======

U-Boot for the Purism Librem 5 development kit

Quick Start
-----------

- Build the ARM Trusted firmware binary
- Get DDR and HDMI firmware
- Build U-Boot

Get and Build the ARM Trusted firmware
--------------------------------------

Note: srctree is U-Boot source directory
Get ATF from: https://source.puri.sm/Librem5/arm-trusted-firmware
branch: librem5

.. code-block:: bash

   $ make PLAT=imx8mq CROSS_COMPILE=aarch64-linux-gnu- bl31
   $ cp build/imx8mq/release/bl31.bin $(builddir)

Get the ddr and display port firmware
-------------------------------------

.. code-block:: bash

   $ wget https://www.nxp.com/lgfiles/NMG/MAD/YOCTO/firmware-imx-8.15.bin
   $ chmod +x firmware-imx-8.15.bin
   $ ./firmware-imx-8.15.bin
   $ cp firmware-imx-8.15/firmware/hdmi/cadence/signed_hdmi_imx8m.bin $(builddir)
   $ cp firmware-imx-8.15/firmware/ddr/synopsys/lpddr4*.bin $(builddir)

Build U-Boot
------------

.. code-block:: bash

   $ export CROSS_COMPILE=aarch64-linux-gnu-
   $ make librem5_devkit_defconfig
   $ make ARCH=arm

Burn the flash.bin
------------------

Write the flash.bin to the eMMC at offset 32KB:

.. code-block:: bash

   $ sudo dd if=flash.bin of=/dev/mmc0 bs=1024 seek=32 conv=notrunc

Reboot the devkit.
