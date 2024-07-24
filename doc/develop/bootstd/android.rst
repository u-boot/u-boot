.. SPDX-License-Identifier: GPL-2.0+:

Android Bootmeth
================

Android provides a mechanism for booting its Operating System from eMMC storage,
described on `source.android.com <https://source.android.com/docs/core/architecture/bootloader>`_.

Android has strong requirements about partitioning layout which are described
`here <https://source.android.com/docs/core/architecture/partitions>`_.
Because multiple partitions are required, this bootmeth only operates on whole mmc
devices which have a valid partition table.

When invoked on a bootdev, this bootmeth searches for the ``misc`` partition in order
to read the *boot mode*, which can be one of following:

Normal
  Boot the regular Android Operating System.

Recovery
  Boot a slimmed down Recovery Operating System. Can be used
  to factory reset the device or to apply system updates.

Bootloader
  Stay in U-Boot and wait for fastboot commands from the host.

After the *boot mode* has been determined, this bootmeth will read the *slot suffix*
from the ``misc`` partition. For details about slots, see
`the AOSP documentation <https://source.android.com/docs/core/ota/ab#slots>`_.

When both the *boot mode* and the *slot suffix* are known, the bootflow is created.

When the bootflow is booted, the bootmeth reads the kernel, the boot arguments and
the vendor ramdisk.
It then boots the kernel using bootm. The relevant devicetree blob is extracted
from the ``boot`` partition based on the ``adtb_idx`` environment variable.

The compatible string "u-boot,android" is used for the driver. It is present
if `CONFIG_BOOTMETH_ANDROID` is enabled.
