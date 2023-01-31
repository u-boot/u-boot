.. SPDX-License-Identifier: GPL-2.0+

U-Boot for ODROID-N2L
=====================

ODROID-N2L is a single board computer manufactured by Hardkernel
Co. Ltd with the following specifications:

 - Amlogic S922X ARM Cortex-A53 dual-core + Cortex-A73 quad-core SoC
 - 4GB DDR4 SDRAM
 - HDMI 2.1 4K/60Hz display
 - 40-pin GPIO header
 - 1 x USB 3.0 Host, 1 x USB USB 2.0 Host
 - eMMC, microSD
 - MIPI DSI Port

Schematics are available on the manufacturer website.

U-Boot compilation
------------------

.. code-block:: bash

    $ export CROSS_COMPILE=aarch64-none-elf-
    $ make odroid-n2l_defconfig
    $ make

Image creation
--------------

For simplified usage, pleaser refer to :doc:`pre-generated-fip` with codename `odroid-n2l`
