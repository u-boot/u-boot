.. SPDX-License-Identifier: GPL-2.0+

phyCORE-i.MX 8M Plus
====================

The phyCORE-i.MX 8M Plus with 2GB of main memory is supported.

Quick Start
-----------

- Build the ARM Trusted firmware binary
- Build the OP-TEE binary (optional)
- Get ddr firmware
- Build U-Boot
- Boot

Build the ARM Trusted firmware binary
-------------------------------------

.. code-block:: bash

   $ git clone https://git.trustedfirmware.org/TF-A/trusted-firmware-a.git
   $ cd trusted-firmware-a
   $ export CROSS_COMPILE=aarch64-linux-gnu-
   $ export IMX_BOOT_UART_BASE=0x30860000
   $ # with optee
   $ make PLAT=imx8mp SPD=opteed bl31
   $ # without optee
   $ make PLAT=imx8mp bl31

Build the OP-TEE binary (optional)
----------------------------------

.. code-block:: bash

   $ git clone https://github.com/OP-TEE/optee_os.git
   $ cd optee_os
   $ make CFG_TEE_BENCHMARK=n \
     CROSS_COMPILE=aarch64-linux-gnu- \
     O=out/arm \
     PLATFORM=imx-mx8mp_phyboard_pollux \
     CFG_TZDRAM_START=0x56000000

Get the ddr firmware
--------------------

.. code-block:: bash

   $ wget https://www.nxp.com/lgfiles/NMG/MAD/YOCTO/firmware-imx-8.19.bin
   $ chmod +x firmware-imx-8.19.bin
   $ ./firmware-imx-8.19.bin

Build U-Boot for SD card
------------------------

Copy binaries
^^^^^^^^^^^^^

.. code-block:: bash

   $ cp <TF-A dir>/build/imx8mp/release/bl31.bin .
   $ cp <OP-TEE dir>/out/arm/core/tee-raw.bin tee.bin
   $ cp firmware-imx-8.19/firmware/ddr/synopsys/lpddr4*.bin .

Build U-Boot
^^^^^^^^^^^^

.. code-block:: bash

   $ make phycore-imx8mp_defconfig
   $ make flash.bin

Flash SD card
^^^^^^^^^^^^^

.. code-block:: bash

   $ sudo dd if=flash.bin of=/dev/sd[x] bs=1024 seek=32 conv=sync
