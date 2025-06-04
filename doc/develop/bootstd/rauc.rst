.. SPDX-License-Identifier: GPL-2.0+:

RAUC Bootmeth
=============

This bootmeth provides a way to locate and run an A/B system with RAUC as its
update client. The booted distro must supply a script on an MMC device
containing the final boot instructions necessary.

This bootmeth assumes a symmetric A/B partition layout, with a separate boot
partition containing the kernel image and another partition for the root
filesystem each. The partition numbers must be specified with
``CONFIG_BOOTMETH_RAUC_PARTITIONS``. The content must be a list of pairs, with
the following syntax: ``1,2 3,4``, where 1 and 3 are the slots' boot partition
and 2 and 4 the slots' root partition.

Each pair of boot and rootfs partition form a "slot". The default order in which
available slots are tried is set through ``CONFIG_BOOTMETH_RAUC_BOOT_ORDER``,
with the left one tried first.

The default number of boot tries of each slot is set by
``CONFIG_BOOTMETH_RAUC_DEFAULT_TRIES``.

In case no valid slot can be found and/or all slots have zero tries left, the
boot order and slot tries are reset to their default values, if
``CONFIG_BOOTMETH_RAUC_RESET_ALL_ZERO_TRIES`` is enabled. This prevents a system
from locking up in the bootloader and tries booting again after a specified
number of tries.

The boot script must be located in each boot partition. The bootmeth searches
for "boot.scr.uimg" first, then "boot.scr" if not found.

When the bootflow is booted, the bootmeth sets these environment variables:

devtype
    device type (e.g. "mmc")

devnum
    device number, corresponding to the device 'sequence' number
    ``dev_seq(dev)``

distro_bootpart
    partition number of the boot partition on the device (numbered from 1)

distro_rootpart
    partition number of the rootfs partition on the device (numbered from 1)

raucargs
    kernel command line arguments needed for RAUC to detect the currently booted
    slot

The script file must be a FIT or a legacy uImage. It is loaded into memory and
executed.

The compatible string "u-boot,distro-rauc" is used for the driver. It is present
if ``CONFIG_BOOTMETH_RAUC`` is enabled.
