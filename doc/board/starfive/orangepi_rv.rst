.. SPDX-License-Identifier: GPL-2.0+

Xunlong OrangePi RV
===================

U-Boot for the OrangePi RV uses the same U-Boot binaries as the VisionFive 2
board. In U-Boot SPL the actual board is detected as a VisionFive2 1.3b due to
a manufacturer problem and having the same EEPROM data as VisionFive2 1.3b.

Device-tree selection
---------------------

U-Boot will set variable $fdtfile to starfive/jh7110-starfive-visionfive-2-v1.3b.dtb

This is sufficient for U-Boot however fails to work correctly with the Linux Kernel.

To overrule this selection the variable can be set manually and saved in the
environment

::

    env set fdtfile starfive/jh7110-orangepi-rv.dtb
    env save

EEPROM modification
-------------------

For advanced users and developers an EEPROM identifier product serial number
beginning with "XOPIRV" will match the OrangePi RV and automatically set the
correct device-tree at U-Boot SPL phase. The procedure for writing EEPROM data
is not detailed here however is similar to that of the Pine64 Star64 and Milk-V
Mars CM. The write-protect disable pads on the Orange Pi RV circuit board
bottom are labeled WP and GND near the M.2 connector.

.. include:: jh7110_common.rst
