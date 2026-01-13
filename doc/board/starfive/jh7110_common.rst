.. SPDX-License-Identifier: GPL-2.0-or-later

Boot source selection
---------------------

The board provides DIP switches to select the device for loading the boot
firmware.

=========== === ===
Boot source SW1 SW2
=========== === ===
UART        OFF OFF
SD-card     ON  OFF
eMMC        OFF ON
SPI flash   ON  ON
=========== === ===

Flashing a new U-Boot version
-----------------------------

U-Boot SPL is provided as file spl/u-boot-spl.bin.normal.out. Main U-Boot is
in file u-boot.itb.

Assuming your new U-Boot version is on partition 1 of an SD-card you could
install it to the SPI flash with:

.. code-block:: console

    sf probe
    load mmc 1:1 $kernel_addr_r u-boot-spl.bin.normal.out
    sf update $kernel_addr_r 0 $filesize
    load mmc 1:1 $kernel_addr_r u-boot.itb
    sf update $kernel_addr_r 0x100000 $filesize

For loading the files from a TFTP server refer to the dhcp and tftpboot
commands.

After updating U-Boot you may want to erase a saved environment and reboot.

.. code-block:: console

    env erase
    reset

Booting from SD-Card
--------------------

The device boot ROM loads U-Boot SPL (u-boot-spl.bin.normal.out) from the
partition with type GUID 2E54B353-1271-4842-806F-E436D6AF6985. You are free
to choose any partition number.

With the default configuration U-Boot SPL loads the U-Boot FIT image
(u-boot.itb) from partition 2 (CONFIG_SYS_MMCSD_RAW_MODE_U_BOOT_PARTITION=0x2).
When formatting it is recommended to use GUID
BC13C2FF-59E6-4262-A352-B275FD6F7172 for this partition.

Booting from eMMC
-----------------

The device boot ROM tries to load U-Boot SPL (u-boot-spl.bin.normal.out) from
sector 0 of the eMMC's main hardware partition. But this conflicts with GPT
partitioning. Fortunately eMMC can alternatively load U-Boot SPL from a backup
position.

For U-Boot SPL (u-boot-spl.bin.normal.out) starting at sector 2048 (position
0x100000) write the following bytes to the eMMC device after GPT partitioning:

======= ========================
Address Bytes
======= ========================
0x0000  40 02 00 00  00 00 10 00
0x0290  40 02 00 00  00 00 10 00
======= ========================

With the default configuration U-Boot SPL loads the U-Boot FIT image
(u-boot.itb) from partition 2 (CONFIG_SYS_MMCSD_RAW_MODE_U_BOOT_PARTITION=0x2).
When formatting it is recommended to use GUID
BC13C2FF-59E6-4262-A352-B275FD6F7172 for this partition.

Booting from UART
-----------------

The boot ROM supports the X-modem protocol to upload
spl/u-boot-spl.bin.normal.out. U-Boot SPL support loading the FIT image
u-boot.itb via the Y-modem protocol.

Due to restrictions of the boot ROM not all X-modem implementations are
compatible. The package tio (https://github.com/tio/tio) has been found to be
usable.

Debug UART
----------

By default the SBI interface is used for the debug UART. But this only works
in main U-Boot. To enable the debug UART in SPL, too, use the following
settings::

    CONFIG_DEBUG_UART=y
    CONFIG_DEBUG_UART_NS16550=y
    CONFIG_DEBUG_UART_BASE=0x10000000
    CONFIG_SPL_DEBUG_UART_BASE=0x10000000
    CONFIG_DEBUG_UART_CLOCK=24000000
    CONFIG_DEBUG_UART_SHIFT=2
