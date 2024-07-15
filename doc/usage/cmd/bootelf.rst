.. SPDX-License-Identifier: GPL-2.0-or-later
.. Copyright 2024, Maxim Moskalets <maximmosk4@gmail.com>

.. index::
   single: bootelf (command)

bootelf command
===============

Synopsis
--------

::

    bootelf [-p|-s] [-d <fdt_addr>] [<image_addr> [<arg>]...]

Description
-----------

The *bootelf* command is used to launch a ELF binary at *image_addr*. If
*image_addr* is not specified, the bootelf command will try to find image in
*image_load_addr* variable (*CONFIG\_SYS\_LOAD\_ADDR* by default).

Args after *image_addr* will be passed to application in common *argc*, *argv*
format.

A command sequence to run a ELF image using FDT might look like

::

    load mmc 0:1 ${loadaddr} /kernel.elf
    load mmc 0:1 ${fdt_addr_r} /soc-board.dtb
    bootelf -d ${fdt_addr_r} ${loadaddr} ${loadaddr}

image_addr
    Address of the ELF binary.

fdt_addr
    Address of the device-tree. This argument in only needed if bootable
    application uses FDT that requires additional setup (like /memory node).

arg
    Any text arguments for bootable application. This is usually the address
    of the device-tree.

Flags:

-p
    Load ELF image via program headers.

-s
    Load ELF image via section headers.

-d
    Setup FDT by address.

Configuration
-------------

The bootelf command is only available if CONFIG_CMD_ELF=y. FDT setup by flag -d
need CONFIG_CMD_ELF_FDT_SETUP=y.
