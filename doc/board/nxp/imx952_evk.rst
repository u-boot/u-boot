.. SPDX-License-Identifier: GPL-2.0+

imx952_evk
=======================

U-Boot for the NXP i.MX952 15x15 LPDDR4X EVK board

Quick Start
-----------

- Get ahab-container.img
- Get DDR PHY Firmware Images
- Get and Build OEI Images
- Get and Build System Manager Image
- Get and Build the ARM Trusted Firmware
- Build the Bootloader Image
- Boot

Get ahab-container.img
--------------------------------------

Note: srctree is U-Boot source directory

.. code-block:: bash

   $ wget https://nl2-nxrm.sw.nxp.com/repository/IMX_Yocto_Internal_Mirror_Recent/firmware-ele-imx-2.0.5-50c4793.bin
   $ sh firmware-ele-imx-2.0.5-50c4793.bin --auto-accept
   $ cp firmware-ele-imx-2.0.5-50c4793/mx952a0-ahab-container.img $(srctree)

Get DDR PHY Firmware Images
--------------------------------------

Note: srctree is U-Boot source directory

.. code-block:: bash

   $ wget https://nl2-nxrm.sw.nxp.com/repository/IMX_Yocto_Internal_Mirror_Recent/firmware-imx-8.32-c0491e4.bin
   $ sh firmware-imx-8.32-c0491e4.bin --auto-accept
   $ cp firmware-imx-8.32-c0491e4/firmware/ddr/synopsys/lpddr4x*v202409.bin $(srctree)

Get and Build OEI Images
--------------------------------------

Note: srctree is U-Boot source directory
Get OEI from: https://github.com/nxp-imx/imx-oei
branch: lf-6.18.2-imx952-er1

.. code-block:: bash

   $ sudo apt -y install make gcc g++-multilib srecord
   $ wget https://developer.arm.com/-/media/Files/downloads/gnu/14.2.rel1/binrel/arm-gnu-toolchain-14.2.rel1-x86_64-arm-none-eabi.tar.xz
   $ tar xvf arm-gnu-toolchain-14.2.rel1-x86_64-arm-none-eabi.tar.xz
   $ export TOOLS=$PWD
   $ git clone https://github.com/nxp-imx/imx-oei/ -b lf-6.18.2-imx952-er1
   $ cd imx-oei
   $ make board=mx952lp4x-15 oei=ddr DEBUG=1 all
   $ cp build/mx952lp4x-15/ddr/oei-m33-ddr.bin $(srctree)

Get and Build System Manager Image
--------------------------------------

Note: srctree is U-Boot source directory
Get System Manager from: https://github.com/nxp-imx/imx-sm
branch: lf-6.18.2-imx952-er1

.. code-block:: bash

   $ sudo apt -y install make gcc g++-multilib srecord
   $ wget https://developer.arm.com/-/media/Files/downloads/gnu/14.2.rel1/binrel/arm-gnu-toolchain-14.2.rel1-x86_64-arm-none-eabi.tar.xz
   $ tar xvf arm-gnu-toolchain-14.2.rel1-x86_64-arm-none-eabi.tar.xz
   $ export TOOLS=$PWD
   $ git clone https://github.com/nxp-imx/imx-sm/ -b lf-6.18.2-imx952-er1
   $ cd imx-sm
   $ make config=mx952evk all
   $ cp build/mx952evk/m33_image.bin $(srctree)

Get and Build the ARM Trusted Firmware
--------------------------------------

Note: srctree is U-Boot source directory
Get ATF from: https://github.com/nxp-imx/imx-atf/
branch: lf-6.18.2-imx952-er1

.. code-block:: bash

   $ export CROSS_COMPILE=aarch64-poky-linux-
   $ unset LDFLAGS
   $ unset AS
   $ git clone https://github.com/nxp-imx/imx-atf/ -b lf-6.18.2-imx952-er1
   $ cd imx-atf
   $ make PLAT=imx952 bl31
   $ cp build/imx952/release/bl31.bin $(srctree)

Build the Bootloader Image
--------------------------

.. code-block:: bash

   $ export CROSS_COMPILE=aarch64-poky-linux-
   $ make imx952_evk_defconfig
   $ make

Copy flash.bin to the MicroSD card:

.. code-block:: bash

   $ sudo dd if=flash.bin of=/dev/sd[x] bs=1k seek=32 conv=fsync

Boot
----

Set i.MX952 boot device to MicroSD card
