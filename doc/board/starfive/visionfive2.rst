.. SPDX-License-Identifier: GPL-2.0+

StarFive VisionFive2
====================

The StarFive VisionFive2 development platform is based on JH7110 and capable
of running Linux.

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

or the configuration variable CONFIG_DEFAULT_FDT_FILE can be used to provide
a default value.

.. include:: jh7110_common.rst
