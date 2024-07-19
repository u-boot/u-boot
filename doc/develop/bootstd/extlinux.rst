.. SPDX-License-Identifier: GPL-2.0+:

Extlinux Bootmeth
=================

`Extlinux <https://uapi-group.org/specifications/specs/boot_loader_specification>`_
(sometimes called syslinux) allows U-Boot to provide a menu of available
operating systems from which the user can choose.

U-Boot includes a parser for the `extlinux.conf` file. It consists primarily of
a list of named operating systems along with the kernel, initial ramdisk and
other settings. The file is stored in the `extlinux/` subdirectory, possibly
under the `boot/` subdirectory. This list of prefixes (``{"/", "/boot"}`` by
default) can be selected with the `filename-prefixes` property in the bootstd
device.

Note that the :doc:`pxelinux` uses the same file format, but in a
network context.

When invoked on a bootdev, this bootmeth searches for the file and creates a
bootflow if found.

When the bootflow is booted, the bootmeth calls ``pxe_setup_ctx()`` to set up
the context, then ``pxe_process()`` to process the file. Depending on the
contents, this may boot an operating system or provide a list of options to
the user, perhaps with a timeout.

The compatible string "u-boot,extlinux" is used for the driver. It is present
if `CONFIG_BOOTMETH_EXTLINUX` is enabled.
