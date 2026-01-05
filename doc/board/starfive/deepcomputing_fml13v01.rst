.. SPDX-License-Identifier: GPL-2.0-or-later

DeepComputing Framework Motherboard (FLM13V01)
==============================================

The DeepComputing Framework motherboard (FLM13V01) can be combined with a
13 inch Framework laptop chassis to provide a complete laptop.

U-Boot for the board uses the same binaries as the VisionFive 2 board.
Currently only serial console output is supported by mainline U-Boot.

Device-tree selection
---------------------

The product ID stored in the board EEPROM is used by U-Boot SPL to select the
right configuration and device-tree from the u-boot.itb FIT image.

Furthermore if variable $fdtfile has not been saved in the environment it is
set based on the product ID to *starfive/jh7110-deepcomputing-fml13v01.dtb*.

To overrule this default the variable can be set manually and saved in the
environment

.. code-block:: console

    setenv fdtfile my_device-tree.dtb
    env save

Power switch
------------

A tiny power switch is located in right upper corner of the board.

Open case detection
-------------------

The board has an open case detection switch. Red lights will flash and the
board will not boot if the switch is not held down.

UART
----

UART 0 is exposed via the side channel contacts SBU1 and SBU2 of the lower,
right USB C connector. A USB C cable and a breakout board are needed for
physical access. It depends on the cable orientation on which of SBU1 and SBU2
you will find RX and TX. The signal voltage is 3.3 V. The baud rate is 115200.

.. include:: jh7110_common.rst
