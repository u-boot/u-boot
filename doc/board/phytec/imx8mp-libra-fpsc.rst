.. SPDX-License-Identifier: GPL-2.0+

Libra i.MX 8M Plus FPSC
=======================

The Libra i.MX 8M Plus FPSC is a SBC based with the phyCORE-i.MX 8M Plus FPSC
SoM.
The phyCORE-i.MX 8M Plus FPSC with 2GB of main memory is supported.

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
          PLAT=imx8mp \
          IMX_BOOT_UART_BASE=0x30a60000 \
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
          PLATFORM=imx-mx8mp_libra_fpsc

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

   $ cp <TF-A dir>/build/imx8mp/release/bl31.bin .
   $ cp <OP-TEE dir>/out/arm/core/tee-raw.bin tee.bin
   $ cp firmware-imx-8.28-994fa14/firmware/ddr/synopsys/lpddr4*.bin .

Build U-Boot
^^^^^^^^^^^^

.. code-block:: bash

   $ make -j $(nproc) \
          CROSS_COMPILE=aarch64-linux-gnu- \
          imx8mp-libra-fpsc_defconfig \
          flash.bin

Flash SD card
^^^^^^^^^^^^^

.. code-block:: bash

   $ sudo dd if=flash.bin of=/dev/sd[x] bs=1024 seek=32 conv=fsync
