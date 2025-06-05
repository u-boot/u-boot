.. SPDX-License-Identifier: GPL-2.0+
.. sectionauthor:: Lukasz Majewski <lukma@denx.de>

BTT devices
===========

Those devices are based on IMX's IMX287 SoC. The description regarding the
**btt** family of boards (i.e. `btt3` and `bttc`) is identical as the one for
the already supported **xea** board.

Building
--------

Make sure that `CROSS_COMPILE` is set appropriately:

.. code-block:: text

 $ make imx28_btt3_defconfig
 $ make -j4 u-boot.sb u-boot.img

Now you should see `u-boot.sb` and `u-boot.img` files in the build directory.

For initial bringup - one can use `uuu` utulity to boot till u-boot prompt
(USB connection with the board is required).

Flashing
--------

Via U-Boot:

.. code-block:: text

 => run update_spl
 => run update_uboot
