.. SPDX-License-Identifier: GPL-2.0+

imx95_navq
==========

U-Boot for the NXP MR-NAVQ95 board

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
----------------------

Note: srctree is U-Boot source directory

.. code-block:: bash

   $ wget https://www.nxp.com/lgfiles/NMG/MAD/YOCTO/firmware-ele-imx-2.0.6-c0b284c.bin
   $ sh firmware-ele-imx-2.0.6-c0b284c.bin --auto-accept
   $ cp firmware-ele-imx-2.0.6-c0b284c/mx95b0-ahab-container.img $(srctree)

Get DDR PHY Firmware Images
---------------------------

Note: srctree is U-Boot source directory

.. code-block:: bash

   $ wget https://www.nxp.com/lgfiles/NMG/MAD/YOCTO/firmware-imx-8.32-1991416.bin
   $ sh firmware-imx-8.32-1991416.bin --auto-accept
   $ cp firmware-imx-8.32-1991416/firmware/ddr/synopsys/lpddr5*v202409.bin $(srctree)

Get and Build OEI Images
------------------------

Note: srctree is U-Boot source directory
Get OEI from: https://github.com/nxp-imx/imx-oei
branch: master

.. code-block:: bash

   $ sudo apt -y install make gcc g++-multilib srecord
   $ mkdir ~/toolchain
   $ wget -P ~/toolchain https://gitlab.arm.com/api/v4/projects/tooling%2Fgnu-toolchains-for-arm/packages/generic/gnu-toolchain/15.3.rel1/arm-gnu-toolchain-15.3.rel1-x86_64-arm-none-eabi.tar.xz
   $ tar xvf arm-gnu-toolchain-15.3.rel1-x86_64-arm-none-eabi.tar.xz -C ~/toolchain
   $ export TOOLS=~/toolchain
   $ git clone -b master https://github.com/nxp-imx/imx-oei.git
   $ cd imx-oei
   $ make TC_VERSION=15.3.rel1 board=mx95lp5 oei=ddr DEBUG=1 r=B0 all
   $ cp build/mx95lp5/ddr/oei-m33-ddr.bin $(srctree)

Get and Build System Manager Image
----------------------------------

Note: srctree is U-Boot source directory
Get System Manager from: https://github.com/NXP-Robotics/robotics-sm
branch: master

.. code-block:: bash

   $ sudo apt -y install make gcc g++-multilib srecord
   $ mkdir ~/toolchain
   $ wget -P ~/toolchain https://gitlab.arm.com/api/v4/projects/tooling%2Fgnu-toolchains-for-arm/packages/generic/gnu-toolchain/15.3.rel1/arm-gnu-toolchain-15.3.rel1-x86_64-arm-none-eabi.tar.xz
   $ tar xvf ~/toolchain/arm-gnu-toolchain-15.3.rel1-x86_64-arm-none-eabi.tar.xz -C ~/toolchain
   $ export TOOLS=~/toolchain
   $ git clone -b robotics-0.7.0 https://github.com/NXP-Robotics/robotics-sm.git
   $ cd robotics-sm
   $ make config=mr-navq95 cfg
   $ make -j$(nproc) TC_VERSION=15.3.rel1 config=mr-navq95 all
   $ cp build/mx95navq/m33_image.bin $(srctree)

Get and Build the ARM Trusted Firmware
--------------------------------------

Note: srctree is U-Boot source directory
Get ATF from: https://github.com/nxp-imx/imx-atf/
branch: lf_v2.14

.. code-block:: bash

   $ sudo apt install gcc-aarch64-linux-gnu
   $ export CROSS_COMPILE=aarch64-linux-gnu-
   $ unset LDFLAGS
   $ unset AS
   $ git clone -b lf_v2.14 https://github.com/nxp-imx/imx-atf.git
   $ cd imx-atf
   $ make PLAT=imx95 bl31
   $ cp build/imx95/release/bl31.bin $(srctree)

Build the Bootloader Image
--------------------------

.. code-block:: bash

   $ sudo apt install gcc-aarch64-linux-gnu
   $ export CROSS_COMPILE=aarch64-linux-gnu-
   $ make imx95_navq_defconfig
   $ make -j$(nproc)

Copy flash.bin to the MicroSD card:

.. code-block:: bash

   $ sudo dd if=flash.bin of=/dev/sd[x] bs=1k seek=32 conv=fsync

Boot
----

Set i.MX95 boot device to MicroSD card.
