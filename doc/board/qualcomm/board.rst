.. SPDX-License-Identifier: GPL-2.0+
.. sectionauthor:: Dzmitry Sankouski <dsankouski@gmail.com>

Qualcomm generic boards
=======================

About this
----------
This document describes how to build and run U-Boot for Qualcomm generic
boards. Right now the generic target supports the Snapdragon 845 SoC, however
it's expected to support more SoCs going forward.

SDM845 - high-end qualcomm chip, introduced in late 2017.
Mostly used in flagship phones and tablets of 2018.

The current boot flow support loading u-boot as an Android boot image via
Qualcomm's UEFI-based ABL (Android) Bootloader. The DTB used by U-Boot will
be appended to the U-Boot image the same way as when booting Linux. U-Boot
will then retrieve the DTB during init. This way the memory layout and KASLR
offset will be populated by ABL.

Installation
------------
Build
^^^^^

	$ ./tools/buildman/buildman -o .output qcom

This will build ``.output/u-boot-nodtb.bin`` using the ``qcom_defconfig``.

Generate FIT image (optional)
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
See doc/uImage.FIT for more details

Pack android boot image
^^^^^^^^^^^^^^^^^^^^^^^
We'll assemble android boot image with ``u-boot-nodtb.bin`` instead of linux kernel,
and FIT image instead of ``initramfs``. Android bootloader expect gzipped kernel
with appended dtb, so let's mimic linux to satisfy stock bootloader.

Boards
------

starqlte
^^^^^^^^

The starqltechn is a production board for Samsung S9 (SM-G9600) phone,
based on the Qualcomm SDM845 SoC.

This device is supported by the common qcom_defconfig.

The DTB is called "sdm845-samsung-starqltechn.dtb"

More information can be found on the `Samsung S9 page`_.

dragonboard845c
^^^^^^^^^^^^^^^

The dragonboard845c is a Qualcomm Robotics RB3 Development Platform, based on
the Qualcomm SDM845 SoC.

This device is supported by the common qcom_defconfig

The DTB is called "sdm845-db845c.dtb"

More information can be found on the `DragonBoard 845c page`_.

qcs404-evb
^^^^^^^^^^

The QCS404 EvB is a Qualcomm Development Platform, based on the Qualcomm QCS404 SoC.

This device is supported by the common qcom_defconfig

The DTB is called "qcs404-evb-4000.dtb"

Building steps
--------------

Steps:

- Build u-boot

As above::

	./tools/buildman/buildman -o .output qcom

Or for db410c (and other boards not supported by the generic target)::

	make CROSS_COMPILE=aarch64-linux-gnu- O=.output dragonboard410c_defconfig
	make O=.output -j$(nproc)

Or for smartphones::

	make CROSS_COMPILE=aarch64-linux-gnu- O=.output qcom_defconfig qcom-phone.config
	make O=.output -j$(nproc)

- gzip u-boot::

	gzip u-boot-nodtb.bin

- Append dtb to gzipped u-boot::

	cat u-boot-nodtb.bin.gz arch/arm/dts/your-board.dtb > u-boot-nodtb.bin.gz-dtb

- If you chose to build a FIT image, A ``qcom.its`` file can be found in ``board/qualcomm/generic/``
  directory. It expects a folder as ``qcom_imgs/`` in the main directory containing pre-built kernel,
  dts and ramdisk images. See ``qcom.its`` for full path to images::

	mkimage -f qcom.its qcom.itb

- Now we've got everything to build android boot image::

	mkbootimg --kernel u-boot-nodtb.bin.gz-dtb --ramdisk db845c.itb \
	--output boot.img --pagesize 4096 --base 0x80000000

Or with no FIT image::

	mkbootimg --kernel u-boot-nodtb.bin.gz-dtb \
	--output boot.img --pagesize 4096 --base 0x80000000

- Flash boot.img using fastboot and erase dtbo to avoid conflicts with our DTB:

  .. code-block:: bash

	fastboot flash boot boot.img
	fastboot erase dtbo

.. _Samsung S9 page: https://en.wikipedia.org/wiki/Samsung_Galaxy_S9
.. _DragonBoard 845c page: https://www.96boards.org/product/rb3-platform/
