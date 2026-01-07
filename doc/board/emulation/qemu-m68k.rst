.. SPDX-License-Identifier: GPL-2.0-or-later
.. Copyright (C) 2025, Kuan-Wei Chiu <visitorckw@gmail.com>

QEMU m68k
=========

QEMU for m68k supports a special 'virt' machine designed for emulation and
virtualization purposes. This document describes how to run U-Boot under it.

The QEMU virt machine models a generic m68k virtual machine with Goldfish
interfaces. It supports the Motorola 68040 CPU architecture.

Building U-Boot
---------------
Set the CROSS_COMPILE environment variable to your m68k toolchain, and run:

.. code-block:: bash

    export CROSS_COMPILE=m68k-linux-gnu-
    make qemu-m68k_defconfig
    make

Running U-Boot
--------------
The minimal QEMU command line to get U-Boot up and running is:

.. code-block:: bash

    qemu-system-m68k -M virt -cpu m68040 -nographic -kernel u-boot

Note that the `-nographic` option is used to redirect the console to stdio,
which connects to the emulated Goldfish TTY device.

Hardware Support
----------------
The following QEMU virt peripherals are supported in U-Boot:

* Goldfish TTY (Serial Console)
* Goldfish RTC (Real Time Clock)
