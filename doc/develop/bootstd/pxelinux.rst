.. SPDX-License-Identifier: GPL-2.0+:

PXE Bootmeth
============

PXE (Preboot eXecution-Environment) provides a way to boot an operating system
over a network interface. The PXE bootmeth supports PXELINUX and allows U-Boot to
provide a menu of possible Operating Systems from which the user can choose.

U-Boot includes a parser for the `extlinux.conf` file described
`here <https://uapi-group.org/specifications/specs/boot_loader_specification>`_.
It consists primarily of a list of named operating systems along with the
kernel, initial ramdisk and other settings. The file is retrieved from a network
server using the TFTP protocol.

When invoked on a bootdev, this bootmeth searches for the file and creates a
bootflow if found. See
`PXELINUX <https://wiki.syslinux.org/wiki/index.php?title=PXELINUX>`_ for
a full description of the search procedure.

When the bootflow is booted, the bootmeth calls ``pxe_setup_ctx()`` to set up
the context, then ``pxe_process()`` to process the file. Depending on the
contents, this may boot an Operating System or provide a list of options to the
user, perhaps with a timeout.

The compatible string "u-boot,extlinux-pxe" is used for the driver. It is
present if `CONFIG_BOOTMETH_EXTLINUX_PXE` is enabled.
