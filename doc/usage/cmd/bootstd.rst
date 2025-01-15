.. SPDX-License-Identifier: GPL-2.0+:

.. index::
   single: bootstd (command)

bootstd command
===============

Synopsis
--------

::

    bootstd images

Description
-----------

The `bootstd` command is used to manage standard boot. At present the only
functionality available is to look at the images which have been loaded, or
could be loaded should a particular bootflow be selected.

See :doc:`/develop/bootstd/index` for more information.

bootflow images
~~~~~~~~~~~~~~~

Lists the available images and their location in memory.

Example
-------

This shows listing images attached to various bootflows, then checking the
content of a few of them::

    => bootflow scan
    => bootflow list
    Showing all bootflows
    Seq  Method       State   Uclass    Part  Name                      Filename
    ---  -----------  ------  --------  ----  ------------------------  ----------------
      0  extlinux     ready   mmc          1  mmc1.bootdev.part_1       /extlinux/extlinux.conf
      1  script       ready   mmc          1  mmc4.bootdev.part_1       /boot/boot.scr
      2  cros         ready   mmc          2  mmc5.bootdev.part_2
      3  cros         ready   mmc          4  mmc5.bootdev.part_4
    ---  -----------  ------  --------  ----  ------------------------  ----------------
    (4 bootflows, 4 valid)
    =>
    => bootstd images
    Seq  Bootflow             Type                  At      Size  Filename
    ---  -------------------  --------------  --------  --------  ----------------
      0  mmc1.bootdev.part_1  extlinux_cfg     8ed5a70       253  /extlinux/extlinux.conf
      1  mmc4.bootdev.part_1  script           8ed9550       c73  /boot/boot.scr
      1  mmc4.bootdev.part_1  logo             8eda2a0      5d42  boot.bmp
      2  mmc5.bootdev.part_2  x86_setup        8ee84d0      3000  setup
      2  mmc5.bootdev.part_2  cmdline          8ee84d0      1000  cmdline
      2  mmc5.bootdev.part_2  kernel                 -      4000  kernel
      3  mmc5.bootdev.part_4  x86_setup        8eeb4e0      3000  setup
      3  mmc5.bootdev.part_4  cmdline          8eeb4e0      1000  cmdline
      3  mmc5.bootdev.part_4  kernel                 -      4000  kernel
    ---  -------------------  --------------  --------  --------  ----------------
    (9 images)
    => md 8eda2a0 10
    08eda2a0: 5d424d42 00000000 008a0000 007c0000  BMB]..........|.
    08eda2b0: 00ac0000 002e0000 00010000 00000018  ................
    08eda2c0: 5cb80000 0b130000 0b130000 00000000  ...\............
    08eda2d0: 00000000 00000000 ff0000ff 00ff0000  ................
    => md 8ee84d0 10
    08ee84d0: 544f4f42 414d495f 2f3d4547 696c6d76  BOOT_IMAGE=/vmli
    08ee84e0: 2d7a756e 35312e35 312d302e 672d3132  nuz-5.15.0-121-g
    08ee84f0: 72656e65 72206369 3d746f6f 7665642f  eneric root=/dev
    08ee8500: 6d766e2f 316e3065 72203170 7571206f  /nvme0n1p1 ro qu

Return value
------------

The return value $? is always 0 (true).


.. BootflowStates_:
