.. SPDX-License-Identifier: GPL-2.0+:

.. index::
   single: load (command)

load command
============

Synopsis
--------

::

    load <interface> [<dev[:part]> [<addr> [<filename> [bytes [pos]]]]]

Description
-----------

The load command is used to read a file from a filesystem into memory.

The number of transferred bytes is saved in the environment variable filesize.
The load address is saved in the environment variable fileaddr.

interface
    interface for accessing the block device (mmc, sata, scsi, usb, ....)

dev
    device number

part
    partition number, defaults to 0 (whole device)

addr
    load address, defaults to environment variable loadaddr or if loadaddr is
    not set to configuration variable CONFIG_SYS_LOAD_ADDR

filename
    path to file, defaults to environment variable bootfile

bytes
    maximum number of bytes to load

pos
    number of bytes to skip

part, addr, bytes, pos are hexadecimal numbers.

Example
-------

::

    => load mmc 0:1 ${kernel_addr_r} snp.efi
    149280 bytes read in 11 ms (12.9 MiB/s)
    =>
    => load mmc 0:1 ${kernel_addr_r} snp.efi 1000000
    149280 bytes read in 9 ms (15.8 MiB/s)
    =>
    => load mmc 0:1 ${kernel_addr_r} snp.efi 1000000 100
    149024 bytes read in 10 ms (14.2 MiB/s)
    =>
    => load mmc 0:1 ${kernel_addr_r} snp.efi 10
    16 bytes read in 1 ms (15.6 KiB/s)
    =>

Null-block-device interfaces
----------------------------

A few ``<interface>`` values have no underlying block device. Their
filesystem implementations directly call a back-end (JTAG
debugger, UBI volume, host running U-Boot under sandbox, ...) and
ignore the ``<dev[:part]>`` field, which may be given as ``-``. So
``load <iface> - <addr> <filename>`` works.

semihosting
    Read files from the host filesystem of an attached JTAG debugger
    using the ARM semihosting protocol. Useful with OpenOCD.
    Built when ``CONFIG_SEMIHOSTING=y``.

ubifs
    Read files from a UBIFS volume that has already been attached
    and mounted with the ``ubi part`` + ``ubifsmount`` commands.
    Built when ``CONFIG_CMD_UBIFS=y``.

sandbox
    Read files from the host filesystem the sandbox binary is
    running under. Available on sandbox builds.

The ``<dev[:part]>`` argument is conventionally written as ``-`` for
these interfaces, to make it visible at the call site that the field
is unused. The filesystem layer never looks at it.

Example::

    => load semihosting - ${kernel_addr_r} kernel.itb
    9437184 bytes read in 412 ms (21.8 MiB/s)
    =>

Configuration
-------------

The load command is only available if CONFIG_CMD_FS_GENERIC=y.

Return value
------------

The return value $? is set to 0 (true) if the file was successfully loaded
even if the number of bytes is less then the specified length.

If an error occurs, the return value $? is set to 1 (false).
