.. SPDX-License-Identifier: GPL-2.0+

U-Boot for Anbernic RGxx3 Devices
=================================

This allows U-Boot to boot the following Anbernic devices:

 - Anbernic RG353M
 - Anbernic RG353P
 - Anbernic RG353V
 - Anbernic RG353VS
 - Anbernic RG503

The correct device is detected automatically by comparing ADC values
from ADC channel 1. In the event of an RG353V, an attempt is then made
to probe for an eMMC and if it fails the device is assumed to be an
RG353VS. Based on the detected device, the environment variables
"board", "board_name", and "fdtfile" are set to the correct values
corresponding to the board which can be read by a boot script to boot
with the correct device tree.

Please note that there are some versions of the RG353 devices with
different panels. Panel auto-detection is planned for a later date.

Building U-Boot
---------------

.. code-block:: bash

    $ export CROSS_COMPILE=aarch64-none-elf-
    $ export BL31=../rkbin/bin/rk35/rk3568_bl31_v1.34.elf
    $ export ROCKCHIP_TPL=../rkbin/bin/rk35/rk3568_ddr_1056MHz_v1.13.bin
    $ make anbernic-rgxx3_defconfig
    $ make

This will build ``u-boot-rockchip.bin`` which can be written to an SD
card.

Image installation
------------------

Write the ``u-boot-rockchip.bin`` to an SD card offset 32kb from the
start.

.. code-block:: bash

    $ dd if=u-boot-rockchip.bin of=/dev/mmcblk0 bs=512 seek=64
