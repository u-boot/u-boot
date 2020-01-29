.. SPDX-License-Identifier: GPL-2.0+
.. sectionauthor:: Andy Shevchenko <andriy.shevchenko@linux.intel.com>

Edison
======

Build Instructions for U-Boot as main bootloader
------------------------------------------------

Simple you can build U-Boot and obtain u-boot.bin::

   $ make edison_defconfig
   $ make all

Updating U-Boot on Edison
-------------------------

By default Intel Edison boards are shipped with preinstalled heavily
patched U-Boot v2014.04. Though it supports DFU which we may be able to
use.

1. Prepare u-boot.bin as described in chapter above. You still need one
   more step (if and only if you have original U-Boot), i.e. run the
   following command::

   $ truncate -s %4096 u-boot.bin

2. Run your board and interrupt booting to U-Boot console. In the console
   call::

   => run do_force_flash_os

3. Wait for few seconds, it will prepare environment variable and runs
   DFU. Run DFU command from the host system::

   $ dfu-util -v -d 8087:0a99 --alt u-boot0 -D u-boot.bin

4. Return to U-Boot console and following hint. i.e. push Ctrl+C, and
   reset the board::

   => reset
