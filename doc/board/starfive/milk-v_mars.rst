.. SPDX-License-Identifier: GPL-2.0+

Milk-V Mars
===========

U-Boot for the Milk-V Mars uses the same U-Boot binaries as the VisionFive 2
board. In U-Boot SPL the actual board is detected and the device-tree patched
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
	make PLATFORM=generic FW_TEXT_START=0x40000000 FW_OPTIONS=0

Now build the U-Boot SPL and U-Boot proper.

.. code-block:: console

	cd <U-Boot-dir>
	make starfive_visionfive2_defconfig
	make OPENSBI=$(opensbi_dir)/build/platform/generic/firmware/fw_dynamic.bin

This will generate the U-Boot SPL image (spl/u-boot-spl.bin.normal.out) as well
as the FIT image (u-boot.itb) with OpenSBI and U-Boot.

Device-tree selection
---------------------

Depending on the board version U-Boot set variable $fdtfile to either
starfive/jh7110-starfive-visionfive-2-v1.2a.dtb or
starfive/jh7110-starfive-visionfive-2-v1.3b.dtb.

To overrule this selection the variable can be set manually and saved in the
environment

::

    setenv fdtfile my_device-tree.dtb
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
