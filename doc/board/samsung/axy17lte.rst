.. SPDX-License-Identifier: GPL-2.0+
.. sectionauthor:: Dzmitry Sankouski <dsankouski@gmail.com>

Samsung 2017 A series phones
============================

About this
----------
This document describes the information about Samsung A(7/5/3) 2017 midrange
phones and u-boot usage steps.

U-Boot can be used as a chain-loaded bootloader to replace Samsung's original SBOOT bootloader.
It is loaded as an Android boot image through SBOOT.

Phone specs
-----------
A3 (SM-320) (a3y17lte)
^^^^^^^^^^^^^^^^^^^^^^
- 4.7 AMOLED display
- Exynos 7870 SoC
- 16GB flash
- 2GB RAM

.. A3 2017 wiki page: https://en.wikipedia.org/wiki/Samsung_Galaxy_A3_(2017)

A5 (SM-520) (a5y17lte)
^^^^^^^^^^^^^^^^^^^^^^
- 5.2 AMOLED display
- Exynos 7880 SoC
- 32GB flash
- 3GB RAM

.. A5 2017 wiki page: https://en.wikipedia.org/wiki/Samsung_Galaxy_A5_(2017)

A7 (SM-720) (a5y17lte)
^^^^^^^^^^^^^^^^^^^^^^
- 5.7 AMOLED display
- Exynos 7880 SoC
- 32GB flash
- 3GB RAM

.. A7 2017 wiki page: https://en.wikipedia.org/wiki/Samsung_Galaxy_A7_(2017)

Installation
------------

Building u-boot
^^^^^^^^^^^^^^^

First, setup ``CROSS_COMPILE`` for aarch64.
Then, build U-Boot for your phone, for example ``a5y17lte``::

  $ export CROSS_COMPILE=<aarch64 toolchain prefix>
  $ make a5y17lte_defconfig
  $ make

This will build ``u-boot.bin`` in the configured output directory.

Payload
^^^^^^^
What is a payload?
""""""""""""""""""
A payload file is a file to be used instead of linux kernel in android boot image.
This file will be loaded into memory, and executed by SBOOT,
and is therefore SBOOT's payload.
It may be pure u-boot (with loading u-boot's payload from flash in mind),
or u-boot + u-boot's payload.

It should be kept in mind, that SBOOT binary patches it's payload after loading
in address range 0x401f8550-0x401f9280. Given SBOOT loads payload to 0x40001000,
a range of 0x1f7550-0x1f8280 (2061648-2065024) in a payload file
will be corrupted after loading to RAM.

Creating payload file
"""""""""""""""""""""
- Assemble FIT image for your kernel
- Create a file for u-boot payload ``touch sboot-payload``
- Write zeroes till 0x200000 address to be sure SBOOT won't corrupt your info
  ``dd if=/dev/zero of=sboot-payload bs=$((0x200000)) count=1``
- Write u-boot to the start of the payload ``dd if=<u-boot.bin path> of=sboot-payload``
- Write FIT image to payload from 0x200000 address
  ``dd if=<FIT image path> of=sboot-payload seek=1 bs=2M``

Creating android boot image
"""""""""""""""""""""""""""
Once payload created, it's time for android image::

  mkbootimg --base 0x40000000 --kernel_offset 0x00000000 --ramdisk_offset 0x01000000 --tags_offset 0x00000100 --pagesize 2048 --second_offset 0x00f00000 --kernel <sboot-payload path> -o uboot.img

Note, that stock Samsung bootloader ignores offsets, set in mkbootimg.

Flashing
""""""""
Flash like regular android boot image.
