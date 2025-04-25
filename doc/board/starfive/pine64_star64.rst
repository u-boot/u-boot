.. SPDX-License-Identifier: GPL-2.0+

Pine64 Star64
=============

U-Boot for the Star64 uses the same U-Boot binaries as the VisionFive 2 board.
In U-Boot SPL the actual board is detected and the device-tree patched
accordingly.

Building
--------

1. Add the RISC-V toolchain to your PATH.
2. Setup ARCH & cross compilation environment variable:

.. code-block:: none

   export CROSS_COMPILE=<riscv64 toolchain prefix>

The M-mode software OpenSBI provides the supervisor binary interface (SBI) and
is responsible for the switch to S-Mode. It is a prerequisite to build U-Boot.
Support for the JH7110 was introduced in OpenSBI 1.2. It is recommended to use
a current release.

.. code-block:: console

	git clone https://github.com/riscv/opensbi.git
	cd opensbi
	make PLATFORM=generic FW_TEXT_START=0x40000000

Now build the U-Boot SPL and U-Boot proper.

.. code-block:: console

	cd <U-Boot-dir>
	make starfive_visionfive2_defconfig
	make OPENSBI=$(opensbi_dir)/build/platform/generic/firmware/fw_dynamic.bin

This will generate the U-Boot SPL image (spl/u-boot-spl.bin.normal.out) as well
as the FIT image (u-boot.itb) with OpenSBI and U-Boot.

Device-tree selection
---------------------

U-Boot will set variable $fdtfile to starfive/jh7110-pine64-star64.dtb.

To overrule this selection the variable can be set manually and saved in the
environment

::

    env set fdtfile my_device-tree.dtb
    env save

or the configuration variable CONFIG_DEFAULT_FDT_FILE can be used to set to
provide a default value.

Preparing the SD-Card
---------------------

The device firmware loads U-Boot SPL (u-boot-spl.bin.normal.out) from the
partition with type GUID 2E54B353-1271-4842-806F-E436D6AF6985. You are free
to choose any partition number.

With the default configuration U-Boot SPL loads the U-Boot FIT image
(u-boot.itb) from partition 2 (CONFIG_SYS_MMCSD_RAW_MODE_U_BOOT_PARTITION=0x2).
When formatting it is recommended to use GUID
BC13C2FF-59E6-4262-A352-B275FD6F7172 for this partition.

The FIT image (u-boot.itb) is a combination of OpenSBI's fw_dynamic.bin,
u-boot-nodtb.bin and the device tree blob.

Format the SD card (make sure the disk has GPT, otherwise use gdisk to switch)

.. code-block:: bash

	sudo sgdisk --clear \
	  --set-alignment=2 \
	  --new=1:4096:8191 --change-name=1:spl --typecode=1:2E54B353-1271-4842-806F-E436D6AF6985\
	  --new=2:8192:16383 --change-name=2:uboot --typecode=2:BC13C2FF-59E6-4262-A352-B275FD6F7172  \
	  --new=3:16384:1654784 --change-name=3:system --typecode=3:EBD0A0A2-B9E5-4433-87C0-68B6B72699C7 \
	  /dev/sdb

Copy U-Boot to the SD card

.. code-block:: bash

	sudo dd if=u-boot-spl.bin.normal.out of=/dev/sdb1
	sudo dd if=u-boot.itb of=/dev/sdb2

	sudo mount /dev/sdb3 /mnt/
	sudo cp u-boot-spl.bin.normal.out /mnt/
	sudo cp u-boot.itb /mnt/
	sudo cp Image.gz /mnt/
	sudo cp initramfs.cpio.gz /mnt/
	sudo cp jh7110-starfive-visionfive-2.dtb /mnt/
	sudo umount /mnt

.. include:: jh7110_common.rst

Serial Number and MAC address issues
------------------------------------

U-Boot requires valid EEPROM data to determine which board-specific fix-up to
apply at runtime. This affects the size of memory initialized, network mac
address numbering, and tuning of the network PHYs.

The Star64 does not currently ship with unique serial numbers per-device.
Devices follow a pattern where the last mac address bytes are a sum of 0x7558
and the serial number (lower port mac0), or a sum of 0x7559 and the serial
number (upper port mac1).

As tested there are several 4gb model units where the serial number and network
mac addresses collide with other devices (serial
``STAR64V1-2310-D004E000-00000005``, MACs ``6c:cf:39:00:75:61``,
``6c:cf:39:00:75:62``)

Some early Star64 boards shipped with an uninitialized EEPROM and no write
protect pull-up resistor in place. Later units of all 4gb and 8gb models
sharing the same serial number in EEPROM data will have this problem that the
network mac addresses are alike between different models and this may be
corrected by defeating the write protect resistor to write new values. As an
alternative to this, it may be worked around by overriding the mac addresses
via U-Boot environment variables.

It is required for any unit having uninitialized EEPROM and recommended for
all later Star64 4gb model units (not properly serialized) to have decided on a
new 6-byte serial number. This serial number should be high enough to
avoid collision with other JH7110 boards and low enough not to overflow i.e.
between ``cafe00`` and ``f00d00``.

Update EEPROM values
^^^^^^^^^^^^^^^^^^^^

1. Prepare EEPROM data in memory

::

	## When there is no error to load existing data:
	mac read_eeprom

	## When there is an error to load non-existing data:
	# "DRAM:  Not a StarFive EEPROM data format - magic error"
	mac initialize

2. Set Star64 values

::

	## Common values
	mac vendor PINE64
	mac pcb_revision c1
	mac bom_revision A

	## Device-specific values
	# Year 2023 week 10 production date, 8GB DRAM, optional eMMC, serial cdef01
	mac product_id STAR64V1-2310-D008E000-00cdef01

	# Last three bytes mac0: 0x7558 + serial number 0xcdef01
	mac mac0_address 6c:cf:39:ce:64:59

	# Last three bytes mac1: 0x7559 + serial number 0xcdef01
	mac mac1_address 6c:cf:39:ce:64:5a

3. Defeat write-protect pull-up resistor (if installed) and write to EEPROM

::

	mac write_eeprom

Set Variables in U-Boot
^^^^^^^^^^^^^^^^^^^^^^^

.. note:: Changing just the serial number will not alter your MAC address

The MAC addresses may be "set" as follows by writing as a custom config to SPI
(Change the last 3 bytes of MAC addreses as appropriate):

::

	env set serial# STAR64V1-2310-D008E000-00cdef01
	env set ethaddr 6c:cf:39:ce:64:59
	env set eth1addr 6c:cf:39:ce:64:5a
	env save
	reset
