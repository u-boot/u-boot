.. SPDX-License-Identifier: GPL-2.0+ OR BSD-3-Clause
.. sectionauthor:: Tom Rini <trini@konsulko.com>

Summary
=======

This document covers various features of the `am335x_evm` default
configuration, some of the related defconfigs, and how to enable hardware
features not present by default in the defconfigs.

Hardware
--------

The binary produced by this board supports, based on parsing of the EEPROM
documented in TI's reference designs:
* AM335x GP EVM
* AM335x EVM SK
* The Beaglebone family of designs

Customization
-------------

Given that all of the above boards are reference platforms (and the
Beaglebone platforms are OSHA), it is likely that this platform code and
configuration will be used as the basis of a custom platform.  It is
worth noting that aside from things such as NAND or MMC only being
required if a custom platform makes use of these blocks, the following
are required, depending on design:

* GPIO is only required if DDR3 power is controlled in a way similar to EVM SK
* SPI is only required for SPI flash, or exposing the SPI bus.

The following blocks are required:

* I2C, to talk with the PMIC and ensure that we do not run afoul of
  errata 1.0.24.

When removing options as part of customization, note that you will likely need
to look at both `include/configs/am335x_evm.h`,
`include/configs/ti_am335x_common.h` and `include/configs/am335x_evm.h` as the
migration to Kconfig is not yet complete.

NAND
----

The AM335x GP EVM ships with a 256MiB NAND available in most profiles.  In
this example to program the NAND we assume that an SD card has been
inserted with the files to write in the first SD slot and that mtdparts
have been configured correctly for the board. All images are first loaded
into memory, then written to NAND.

Step-1: Building u-boot for NAND boot
	Set following CONFIGxx options for NAND device.
	CONFIG_SYS_NAND_PAGE_SIZE	number of main bytes in NAND page
	CONFIG_SYS_NAND_OOBSIZE		number of OOB bytes in NAND page
	CONFIG_SYS_NAND_BLOCK_SIZE	number of bytes in NAND erase-block
	CONFIG_SYS_NAND_ECCPOS		ECC map for NAND page
	CONFIG_NAND_OMAP_ECCSCHEME	(refer doc/README.nand)

Step-2: Flashing NAND via MMC/SD

.. code-block:: text

	# select BOOTSEL to MMC/SD boot and boot from MMC/SD card
	U-Boot # mmc rescan
	# erase flash
	U-Boot # nand erase.chip
	U-Boot # env default -f -a
	U-Boot # saveenv
	# flash MLO. Redundant copies of MLO are kept for failsafe
	U-Boot # load mmc 0 0x82000000 MLO
	U-Boot # nand write 0x82000000 0x00000 0x20000
	U-Boot # nand write 0x82000000 0x20000 0x20000
	U-Boot # nand write 0x82000000 0x40000 0x20000
	U-Boot # nand write 0x82000000 0x60000 0x20000
	# flash u-boot.img
	U-Boot # load mmc 0 0x82000000 u-boot.img
	U-Boot # nand write 0x82000000 0x80000 0x60000
	# flash kernel image
	U-Boot # load mmc 0 0x82000000 uImage
	U-Boot # nand write 0x82000000 ${nandsrcaddr} ${nandimgsize}
	# flash filesystem image
	U-Boot # load mmc 0 0x82000000 filesystem.img
	U-Boot # nand write 0x82000000 ${loadaddress} 0x300000

Step-3: Set BOOTSEL pin to select NAND boot, and POR the device.
	The device should boot from images flashed on NAND device.


Falcon Mode
-----------

The default build includes "Falcon Mode" (see doc/README.falcon) via NAND,
eMMC (or raw SD cards) and FAT SD cards.  Our default behavior currently is
to read a 'c' on the console while in SPL at any point prior to loading the
OS payload (so as soon as possible) to opt to booting full U-Boot.  Also
note that while one can program Falcon Mode "in place" great care needs to
be taken by the user to not 'brick' their setup.  As these are all eval
boards with multiple boot methods, recovery should not be an issue in this
worst-case however.

Falcon Mode: eMMC
-----------------

The recommended layout in this case is:

.. code-block:: text

	MMC BLOCKS      |--------------------------------| LOCATION IN BYTES
	0x0000 - 0x007F : MBR or GPT table               : 0x000000 - 0x020000
	0x0080 - 0x00FF : ARGS or FDT file               : 0x010000 - 0x020000
	0x0100 - 0x01FF : SPL.backup1 (first copy used)  : 0x020000 - 0x040000
	0x0200 - 0x02FF : SPL.backup2 (second copy used) : 0x040000 - 0x060000
	0x0300 - 0x06FF : U-Boot                         : 0x060000 - 0x0e0000
	0x0700 - 0x08FF : U-Boot Env + Redundant         : 0x0e0000 - 0x120000
	0x0900 - 0x28FF : Kernel                         : 0x120000 - 0x520000

Note that when we run 'spl export' it will prepare to boot the kernel.
This includes relocation of the uImage from where we loaded it to the entry
point defined in the header.  As these locations overlap by default, it
would leave us with an image that if written to MMC will not boot, so
instead of using the loadaddr variable we use 0x81000000 in the following
example.  In this example we are loading from the network, for simplicity,
and assume a valid partition table already exists and 'mmc dev' has already
been run to select the correct device.  Also note that if you previously
had a FAT partition (such as on a Beaglebone Black) it is not enough to
write garbage into the area, you must delete it from the partition table
first.

.. code-block:: text

	# Ensure we are able to talk with this mmc device
	U-Boot # mmc rescan
	U-Boot # tftp 81000000 am335x/MLO
	# Write to two of the backup locations ROM uses
	U-Boot # mmc write 81000000 100 100
	U-Boot # mmc write 81000000 200 100
	# Write U-Boot to the location set in the config
	U-Boot # tftp 81000000 am335x/u-boot.img
	U-Boot # mmc write 81000000 300 400
	# Load kernel and device tree into memory, perform export
	U-Boot # tftp 81000000 am335x/uImage
	U-Boot # run findfdt
	U-Boot # tftp ${fdtaddr} am335x/${fdtfile}
	U-Boot # run mmcargs
	U-Boot # spl export fdt 81000000 - ${fdtaddr}
	# Write the updated device tree to MMC
	U-Boot # mmc write ${fdtaddr} 80 80
	# Write the uImage to MMC
	U-Boot # mmc write 81000000 900 2000

Falcon Mode: FAT SD cards
-------------------------

In this case the additional file is written to the filesystem.  In this
example we assume that the uImage and device tree to be used are already on
the FAT filesystem (only the uImage MUST be for this to function
afterwards) along with a Falcon Mode aware MLO and the FAT partition has
already been created and marked bootable:

.. code-block:: text

	U-Boot # mmc rescan
	# Load kernel and device tree into memory, perform export
	U-Boot # load mmc 0:1 ${loadaddr} uImage
	U-Boot # run findfdt
	U-Boot # load mmc 0:1 ${fdtaddr} ${fdtfile}
	U-Boot # run mmcargs
	U-Boot # spl export fdt ${loadaddr} - ${fdtaddr}

This will print a number of lines and then end with something like:

.. code-block:: text

           Using Device Tree in place at 80f80000, end 80f85928
           Using Device Tree in place at 80f80000, end 80f88928

So then you:

.. code-block:: text

        U-Boot # fatwrite mmc 0:1 0x80f80000 args 8928

Falcon Mode: NAND
-----------------

In this case the additional data is written to another partition of the
NAND.  In this example we assume that the uImage and device tree to be are
already located on the NAND somewhere (such as filesystem or mtd partition)
along with a Falcon Mode aware MLO written to the correct locations for
booting and mtdparts have been configured correctly for the board:

.. code-block:: text

	U-Boot # nand read ${loadaddr} kernel
	U-Boot # load nand rootfs ${fdtaddr} /boot/am335x-evm.dtb
	U-Boot # run nandargs
	U-Boot # spl export fdt ${loadaddr} - ${fdtaddr}
	U-Boot # nand erase.part u-boot-spl-os
	U-Boot # nand write ${fdtaddr} u-boot-spl-os
