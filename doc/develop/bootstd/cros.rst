.. SPDX-License-Identifier: GPL-2.0+:

ChromiumOS Bootmeth
===================

ChromiumOS provides a mechanism for booting its Operating System from a block
device, described
`here <https://www.chromium.org/chromium-os/chromiumos-design-docs/verified-boot/>`_.

U-Boot includes support for reading the associated data structures from the
device and identifying a bootable ChromiumOS image. This structure includes the
kernel itself, boot arguments (kernel command line), as well as the x86 setup
block (for x86 only).

When invoked on a bootdev, this bootmeth searches for kernel partitions with
the appropriate GUID (Globally Unique Identifier). When found, the information
is loaded and a bootflow is created.

When the bootflow is booted, the bootmeth reads the kernel and boot arguments.
It then boots the kernel using zboot (on x86) or bootm (on ARM). The boot
arguments are adjusted to replace `%U` with the UUID of the selected kernel
partition. This results in the correct root disk being used, which is the next
partition after the kernel partition.

For ARM, a :doc:`/usage/fit/index` is used. The `CONFIG_FIT_BEST_MATCH` option
must be enabled for U-Boot to select the correct devicetree to boot with.

Note that a ChromiumOS image typically has two copies of the Operating System,
each with its own kernel and root disk. There is no initial ramdisk (initrd).
This means that this bootmeth typically locates two separate images.

The compatible string "u-boot,cros" is used for the driver. It is present
if `CONFIG_BOOTMETH_CROS` is enabled.
