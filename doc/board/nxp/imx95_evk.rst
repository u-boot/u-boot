.. SPDX-License-Identifier: GPL-2.0+

imx95_evk
=======================

U-Boot for the NXP i.MX95 19x19 EVK board

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

   $ wget https://www.nxp.com/lgfiles/NMG/MAD/YOCTO/firmware-ele-imx-2.0.2-89161a8.bin
   $ sh firmware-ele-imx-2.0.2-89161a8.bin --auto-accept
   $ cp firmware-ele-imx-2.0.2-89161a8/mx95b0-ahab-container.img $(srctree)

Get DDR PHY Firmware Images
--------------------------------------

Note: srctree is U-Boot source directory

.. code-block:: bash

   $ wget https://www.nxp.com/lgfiles/NMG/MAD/YOCTO/firmware-imx-8.28-994fa14.bin
   $ sh firmware-imx-8.28-994fa14.bin --auto-accept
   $ cp firmware-imx-8.28-994fa14/firmware/ddr/synopsys/lpddr5*v202409.bin $(srctree)

Get and Build OEI Images
--------------------------------------

Note: srctree is U-Boot source directory
Get OEI from: https://github.com/nxp-imx/imx-oei
branch: master

.. code-block:: bash

   $ sudo apt -y install make gcc g++-multilib srecord
   $ wget https://developer.arm.com/-/media/Files/downloads/gnu/13.3.rel1/binrel/arm-gnu-toolchain-13.3.rel1-x86_64-arm-none-eabi.tar.xz
   $ tar xvf arm-gnu-toolchain-13.3.rel1-x86_64-arm-none-eabi.tar.xz
   $ export TOOLS=$PWD
   $ git clone -b master https://github.com/nxp-imx/imx-oei.git
   $ cd imx-oei
   $ make board=mx95lp5 oei=ddr DEBUG=1
   $ cp build/mx95lp5/ddr/oei-m33-ddr.bin $(srctree)

   $ make board=mx95lp5 oei=tcm DEBUG=1
   $ cp build/mx95lp5/tcm/oei-m33-tcm.bin $(srctree)

Get and Build System Manager Image
--------------------------------------

Note: srctree is U-Boot source directory
Get System Manager from: https://github.com/nxp-imx/imx-sm
branch: master

.. code-block:: bash

   $ sudo apt -y install make gcc g++-multilib srecord
   $ wget https://developer.arm.com/-/media/Files/downloads/gnu/13.3.rel1/binrel/arm-gnu-toolchain-13.3.rel1-x86_64-arm-none-eabi.tar.xz
   $ tar xvf arm-gnu-toolchain-13.3.rel1-x86_64-arm-none-eabi.tar.xz
   $ export TOOLS=$PWD
   $ git clone -b master https://github.com/nxp-imx/imx-sm.git
   $ cd imx-sm
   $ make config=mx95evk all
   $ cp build/mx95evk/m33_image.bin $(srctree)

Get and Build the ARM Trusted Firmware
--------------------------------------

Note: srctree is U-Boot source directory
Get ATF from: https://github.com/nxp-imx/imx-atf/
branch: lf_v2.10

.. code-block:: bash

   $ export CROSS_COMPILE=aarch64-poky-linux-
   $ unset LDFLAGS
   $ git clone -b lf_v2.10 https://github.com/nxp-imx/imx-atf.git
   $ cd imx-atf
   $ make PLAT=imx95 bl31
   $ cp build/imx95/release/bl31.bin $(srctree)

Build the Bootloader Image
--------------------------

.. code-block:: bash

   $ export CROSS_COMPILE=aarch64-poky-linux-
   $ make imx95_19x19_evk_defconfig
   $ make

Copy imx-boot-imx95.bin to the MicroSD card:

.. code-block:: bash

   $ sudo dd if=flash.bin of=/dev/sd[x] bs=1k seek=32 conv=fsync

Boot
----

Set i.MX95 boot device to MicroSD card
