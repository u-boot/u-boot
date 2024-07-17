.. SPDX-License-Identifier: GPL-2.0+:

Script Bootmeth
===============

This bootmeth provides a way to locate and run a script on a block or network
device. It can also support SPI flash.

For a block device the file is read from the selected partition, which must use
a supported filesystem. The subdirectory to search in is defined by the bootstd
list of prefixes (``{"/", "/boot"}`` by default) and can be adjust with the
`filename-prefixes` property in the bootstd device.

For a network device, the filename is obtained from the `boot_script_dhcp`
environment variable and the file is read using tftp. It must be in the
top-level directory of the tftp server.

In either case (file or network), the bootmeth searches for the file and creates
a bootflow if found. The bootmeth searches for "boot.scr.uimg" first, then
"boot.scr" if not found.

For SPI flash, a script is read from flash using the offset provided by the
"script_offset_f" environment variable.

Some attempt is made to identify the Operating System: so far this only detects
an `Armbian <https://www.armbian.com>`_
distro. For block devices, if a file called "boot.bmp" exists in the same
directory then it is used as the bootflow logo.

When the bootflow is booted, the bootmeth sets these environment variables:

    devtype
        device type (e.g. "usb", "mmc", "ethernet" or "spi_flash")

    devnum
        device number, corresponding to the device 'sequence' number
        ``dev_seq(dev)``

    distro_bootpart
        (block devices only) partition number on the device (numbered from 1)

    prefix
        prefix used to find the file

    mmc_bootdev
        device number (same as `devnum`), set for sunxi mmc devices only

The script file must be a FIT or a legacy uImage. It is loaded into memory and
executed.

The compatible string "u-boot,script" is used for the driver. It is present
if `CONFIG_BOOTMETH_SCRIPT` is enabled.
