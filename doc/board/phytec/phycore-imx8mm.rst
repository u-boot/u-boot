.. SPDX-License-Identifier: GPL-2.0+

phyCORE-i.MX 8M Mini
====================

The phyCORE-i.MX 8M Mini with 2GB of main memory is supported.

Quick Start
-----------

- Build the ARM Trusted firmware binary
- Build the OP-TEE binary
- Get ddr firmware
- Build U-Boot
- Boot

Build the ARM Trusted firmware binary
-------------------------------------

.. code-block:: bash

   $ git clone https://git.trustedfirmware.org/TF-A/trusted-firmware-a.git
   $ cd trusted-firmware-a
   $ make -j $(nproc) \
          CROSS_COMPILE=aarch64-linux-gnu- \
          PLAT=imx8mm \
          IMX_BOOT_UART_BASE=0x30880000 \
          BL32_BASE=0x7e000000 \
          SPD=opteed \
          bl31

Build the OP-TEE binary
-----------------------

.. code-block:: bash

   $ git clone https://github.com/OP-TEE/optee_os.git
   $ cd optee_os
   $ make -j $(nproc) \
          CROSS_COMPILE=aarch64-linux-gnu- \
          O=out/arm \
          PLATFORM=imx-mx8mm_phyboard_polis

Get the ddr firmware
--------------------

.. code-block:: bash

   $ wget https://www.nxp.com/lgfiles/NMG/MAD/YOCTO/firmware-imx-8.28-994fa14.bin
   $ chmod +x firmware-imx-8.28-994fa14.bin
   $ ./firmware-imx-8.28-994fa14.bin

Build U-Boot for SD card
------------------------

Copy binaries
^^^^^^^^^^^^^

.. code-block:: bash

   $ cp <TF-A dir>/build/imx8mm/release/bl31.bin .
   $ cp <OP-TEE dir>/out/arm/core/tee-raw.bin tee.bin
   $ cp firmware-imx-8.28-994fa14/firmware/ddr/synopsys/lpddr4*.bin .

Build U-Boot
^^^^^^^^^^^^

.. code-block:: bash

   $ make -j $(nproc) \
          CROSS_COMPILE=aarch64-linux-gnu- \
          phycore-imx8mm_defconfig \
          flash.bin

Flash SD card
^^^^^^^^^^^^^

.. code-block:: bash

   $ sudo dd if=flash.bin of=/dev/sd[x] bs=1024 seek=33 conv=sync
