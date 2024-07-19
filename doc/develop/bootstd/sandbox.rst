.. SPDX-License-Identifier: GPL-2.0+:

Sandbox Bootmeth
================

The sandbox bootmeth is only used for testing. It does not provide any facility
for booting an OS. While sandbox can do all the processing before the actual
boot, it is not connected in this bootmeth.

When invoked on a bootdev, this bootmeth pretends to find a bootflow and creates
the associated structure.

When the bootflow is booted, the bootmeth returns `-ENOTSUPP` indicating that it
is not supported.

The compatible string "u-boot,sandbox-bootmeth" is used for the driver. It is present
if `CONFIG_BOOTMETH_SANDBOX` is enabled.
