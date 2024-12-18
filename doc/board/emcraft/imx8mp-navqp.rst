.. SPDX-License-Identifier: GPL-2.0-or-later
.. sectionauthor:: Gilles Talis <gilles.talis@gmail.com>

i.MX8M Plus NavQ+ Board
=======================

U-Boot for the EmCraft Systems i.MX8M Plus NavQ+ board

Quick Start
-----------

- Build the ARM trusted firmware binary
- Get the DDR firmware
- Build U-Boot
- Flash to eMMC
- Boot

Get and Build the ARM Trusted Firmware (Trusted Firmware A)
-----------------------------------------------------------

.. code-block:: bash

    $ echo "Downloading and building TF-A..."
    $ git clone -b lts-v2.10 https://git.trustedfirmware.org/TF-A/trusted-firmware-a.git
    $ cd trusted-firmware-a

Then build ATF (TF-A):

.. code-block:: bash

    $ export CROSS_COMPILE=aarch64-linux-gnu-
    $ make PLAT=imx8mp bl31
    $ cp build/imx8mp/release/bl31.bin ../

Get the DDR Firmware
--------------------

.. code-block:: bash

    $ cd ..
    $ wget https://www.nxp.com/lgfiles/NMG/MAD/YOCTO/firmware-imx-8.23.bin
    $ chmod +x firmware-imx-8.23.bin
    $ ./firmware-imx-8.23.bin
    $ cp firmware-imx-8.23/firmware/ddr/synopsys/lpddr4*_202006.bin ./

Build U-Boot
------------

.. code-block:: bash

    $ export CROSS_COMPILE=aarch64-linux-gnu-
    $ make imx8mp_navqp_defconfig
    $ make

Burn the flash.bin to the MicroSD card at offset 32KB:

.. code-block:: bash

   $ sudo dd if=flash.bin of=/dev/sd[x] bs=1K seek=32 conv=notrunc; sync

Boot
----

Set Boot switch to SD boot
Use /dev/ttyUSB0 for U-Boot console
