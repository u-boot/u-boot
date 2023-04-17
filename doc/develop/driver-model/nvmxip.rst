.. SPDX-License-Identifier: GPL-2.0+

NVM XIP Block Storage Emulation Driver
=======================================

Summary
-------

Non-Volatile Memory devices with addressable memory (e.g: QSPI NOR flash) could
be used for block storage needs (e.g: parsing a GPT layout in a raw QSPI NOR flash).

The NVMXIP Uclass provides this functionality and can be used for any 64-bit platform.

The NVMXIP Uclass provides the following drivers:

      nvmxip-blk block driver:

        A generic block driver allowing to read from the XIP flash.
	The driver belongs to UCLASS_BLK.
	The driver implemented by drivers/mtd/nvmxip/nvmxip.c

      nvmxip Uclass driver:

        When a device is described in the DT and associated with UCLASS_NVMXIP,
        the Uclass creates a block device and binds it with the nvmxip-blk.
	The Uclass driver implemented by drivers/mtd/nvmxip/nvmxip-uclass.c

    The implementation is generic and can be used by different platforms.

Supported hardware
--------------------------------

Any 64-bit plaform.

Configuration
----------------------

config NVMXIP
	  This option allows the emulation of a block storage device
	  on top of a direct access non volatile memory XIP flash devices.
	  This support provides the read operation.
	  This option provides the block storage driver nvmxip-blk which
	  handles the read operation. This driver is HW agnostic and can support
	  multiple flash devices at the same time.

Contributors
------------
   * Abdellatif El Khlifi <abdellatif.elkhlifi@arm.com>
