.. SPDX-License-Identifier: GPL-2.0+
.. Copyright (C) 2024 Jiaxun Yang <jiaxun.yang@flygoat.com>

QEMU Xtensa
===========

QEMU for Xtensa supports a special 'virt' machine designed for emulation and
virtualization purposes. This document describes how to run U-Boot under it.

The QEMU virt machine models a generic Xtensa virtual machine with PCI Bus
and Xtensa ISS simcall semihosting support. It supports many different Xtensa
CPU configuration. Currently, only dc233c variant is tested against U-Boot.

Building U-Boot
---------------
Set the CROSS_COMPILE environment variable as usual, and run:

    make qemu-xtensa-dc233c_defconfig
    make

Note that Xtensa's toolchain is bounded to CPU configuration, you must use
the toolchain built for exactly the same CPU configuration as you selected
in U-Boot.

Running U-Boot
--------------
The minimal QEMU command line to get U-Boot up and running is:

    qemu-system-xtensa -nographic -machine virt -cpu dc233c -semihosting -kernel ./u-boot.elf

You many change cpu option to match your U-Boot CPU type configuration.
semihosting option is mandatory because this is the only way to interact
with U-Boot in command line.
