.. SPDX-License-Identifier: GPL-2.0-or-later

DeepComputing Framework Motherboard (FLM13V01)
==============================================

The DeepComputing Framework motherboard (FLM13V01) can be combined with a
13 inch Framework laptop chassis to provide a complete laptop.

U-Boot for the board uses the same binaries as the VisionFive 2 board.
Currently only serial console output is supported by mainline U-Boot.

Building
--------

Setup the cross compilation environment variable:

.. code-block:: bash

   export CROSS_COMPILE=riscv64-linux-gnu-

The M-mode software OpenSBI provides the supervisor binary interface (SBI) and
is responsible for the switch to S-Mode. It is a prerequisite for building
U-Boot. Support for the JH7110 was introduced in OpenSBI 1.2. It is recommended
to use a current release.

.. code-block:: bash

    git clone https://github.com/riscv/opensbi.git
    cd opensbi
    make PLATFORM=generic FW_TEXT_START=0x40000000 FW_OPTIONS=0
    export OPENSBI="$(pwd)/build/platform/generic/firmware/fw_dynamic.bin"

Now build U-Boot SPL and main U-Boot.

.. code-block:: bash

    cd <U-Boot-dir>
    make starfive_visionfive2_defconfig
    make

This will generate the U-Boot SPL image (spl/u-boot-spl.bin.normal.out) as well
as the FIT image (u-boot.itb) with OpenSBI, U-Boot, and device-trees.

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
