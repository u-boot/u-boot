.. SPDX-License-Identifier: GPL-2.0+

Milk-V Mars
===========

U-Boot for the Milk-V Mars uses the same U-Boot binaries as the VisionFive 2
board. In U-Boot SPL the actual board is detected and the device-tree patched
accordingly.

Device-tree selection
---------------------

Depending on the board version U-Boot set variable $fdtfile to either
starfive/jh7110-starfive-visionfive-2-v1.2a.dtb or
starfive/jh7110-starfive-visionfive-2-v1.3b.dtb.

To overrule this selection the variable can be set manually and saved in the
environment

::

    setenv fdtfile my_device-tree.dtb
    env save

or the configuration variable CONFIG_DEFAULT_FDT_FILE can be used to set to
provide a default value.

.. include:: jh7110_common.rst
