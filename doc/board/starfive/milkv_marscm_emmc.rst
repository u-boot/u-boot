.. SPDX-License-Identifier: GPL-2.0+

Milk-V Mars CM (eMMC)
=====================

U-Boot for the Mars CM uses the same U-Boot binaries as the VisionFive 2 board.
Devicetree selection is made in U-Boot SPL at runtime, dependent on the content
of EEPROM.

Device-tree selection
---------------------

U-Boot will set variable $fdtfile to starfive/jh7110-milkv-marscm-emmc.dtb
dependent on the content of EEPROM.

To overrule this selection the variable can be set manually and saved in the
environment

::

    env set fdtfile my_device-tree.dtb
    env save

.. include:: jh7110_common.rst
