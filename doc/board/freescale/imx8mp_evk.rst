.. SPDX-License-Identifier: GPL-2.0+

imx8mp_evk
==========

U-Boot for the NXP i.MX8MP EVK board

Quick Start
-----------

- Build the ARM Trusted firmware binary
- Get the firmware-imx package
- Build U-Boot
- Boot

Get and Build the ARM Trusted firmware
--------------------------------------

Note: $(srctree) is the U-Boot source directory
Get ATF from: https://source.codeaurora.org/external/imx/imx-atf
branch: imx_5.4.3_2.0.0

.. code-block:: bash

   $ make PLAT=imx8mp bl31
   $ cp build/imx8mp/release/bl31.bin $(srctree)

Get the ddr firmware
--------------------

.. code-block:: bash

   $ wget https://www.nxp.com/lgfiles/NMG/MAD/YOCTO/firmware-imx-8.7.bin
   $ chmod +x firmware-imx-8.7.bin
   $ ./firmware-imx-8.7.bin
   $ cp firmware-imx-8.7/firmware/ddr/synopsys/lpddr4_pmu_train_1d_dmem_201904.bin $(srctree)/lpddr4_pmu_train_1d_dmem.bin
   $ cp firmware-imx-8.7/firmware/ddr/synopsys/lpddr4_pmu_train_1d_imem_201904.bin $(srctree)/lpddr4_pmu_train_1d_imem.bin
   $ cp firmware-imx-8.7/firmware/ddr/synopsys/lpddr4_pmu_train_2d_dmem_201904.bin $(srctree)/lpddr4_pmu_train_2d_dmem.bin
   $ cp firmware-imx-8.7/firmware/ddr/synopsys/lpddr4_pmu_train_2d_imem_201904.bin $(srctree)/lpddr4_pmu_train_2d_imem.bin

Build U-Boot
------------

.. code-block:: bash

   $ export CROSS_COMPILE=aarch64-poky-linux-
   $ make imx8mp_evk_defconfig
   $ export ATF_LOAD_ADDR=0x960000
   $ make flash.bin

Burn the flash.bin to the MicroSD card at offset 32KB:

.. code-block:: bash

   $sudo dd if=flash.bin of=/dev/sd[x] bs=1K seek=32 conv=notrunc; sync

Boot
----

Set Boot switch to SD boot
Use /dev/ttyUSB2 for U-Boot console
