.. SPDX-License-Identifier: GPL-2.0+

U-Boot for ODROID-GO-ULTRA
==========================

The Odroid Go Ultra is a portable gaming device with the following
characteristics:

 - Amlogic S922X SoC
 - RK817 & RK818 PMICs
 - 2GiB LPDDR4
 - On board 16GiB eMMC
 - Micro SD Card slot
 - 5inch 854×480 MIPI-DSI TFT LCD
 - Earphone stereo jack, 0.5Watt 8Ω Mono speaker
 - Li-Polymer 3.7V/4000mAh Battery
 - USB-A 2.0 Host Connector
 - x16 GPIO Input Buttons
 - 2x ADC Analog Joysticks
 - USB-C Port for USB2 Device and Charging

U-Boot compilation
------------------

.. code-block:: bash

    $ export CROSS_COMPILE=aarch64-none-elf-
    $ make odroid-go-ultra_defconfig
    $ make

Image creation
--------------

Pleaser refer to :doc:`pre-generated-fip` with codename `odroid-go-ultra`
