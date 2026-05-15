.. SPDX-License-Identifier: GPL-2.0

P2041-RDB Board Overview
========================

The P2041 Processor combines four Power Architecture processor cores
with high-performance datapath acceleration architecture(DPAA), CoreNet
fabric infrastructure, as well as network and peripheral bus interfaces
required for networking, telecom/datacom, wireless infrastructure, and
military/aerospace applications.

P2041RDB board is a quad core platform supporting the P2041 processor
of QorIQ DPAA series.

Boot from NOR flash
===================

1. Build image::

    make P2041RDB_config
    make all

2. Program image::

    => tftp 1000000 u-boot.bin
    => protect off all
    => erase eff40000 efffffff
    => cp.b 1000000 eff40000 c0000

3. Program RCW::

    => tftp 1000000 rcw.bin
    => protect off all
    => erase e8000000 e801ffff
    => cp.b 1000000 e8000000 50

4. Program FMAN Firmware ucode::

    => tftp 1000000 ucode.bin
    => protect off all
    => erase eff00000 eff3ffff
    => cp.b 1000000 eff00000 2000

5. Change DIP-switch to SW1[1-5] = 10110. Note: 1 stands for 'on', 0 stands for 'off'

Boot from SDCard
================

1. Build image::

    make P2041RDB_SDCARD_config
    make all

2. Program the PBL image to SDCard::

    => tftp 1000000 u-boot.pbl
    => mmc info
    => mmc write 1000000 8 672

3. Program FMAN Firmware ucode::

    => tftp 1000000 ucode.bin
    => mmc write 1000000 690 10

4. Change DIP-switch to SW1[1-5] = 01100. Note: 1 stands for 'on', 0 stands for 'off'

Boot from SPI flash
===================

1. Build image::

    make P2041RDB_SPIFLASH_config
    make all

2. Program the PBL image to SPI flash::

    => tftp 1000000 u-boot.pbl
    => sf probe 0
    => sf update $fileaddr 0 $filesize

3. Program FMAN Firmware ucode::

    => tftp 1000000 ucode.bin
    => sf update $fileaddr 110000 $filesize

4. Change DIP-switch SW1[1-5] = 10100. Note: 1 stands for 'on', 0 stands for 'off'

Device tree support and how to enable it for different configs
--------------------------------------------------------------

Device tree support is available for p2041rdb for below mentioned boot,

1. NOR Boot
2. NAND Boot
3. SD Boot
4. SPIFLASH Boot

To enable device tree support for other boot, below configs need to be
enabled in relative defconfig file,

1. CONFIG_DEFAULT_DEVICE_TREE="p2041rdb" (Change default device tree name if required)
2. CONFIG_OF_CONTROL
3. CONFIG_MPC85XX_HAVE_RESET_VECTOR if reset vector is located at
   CFG_RESET_VECTOR_ADDRESS - 0xffc

CPLD command
============

The CPLD is used to control the power sequence and some serdes lane
mux function::

  cpld reset			 - hard reset to default bank
  cpld reset altbank		 - reset to alternate bank
  cpld lane_mux <lane> <mux_value> - set multiplexed lane pin
                lane 6: 0 -> slot1 (Default)
                        1 -> SGMII
                lane a: 0 -> slot2 (Default)
                        1 -> AURORA
                lane c: 0 -> slot2 (Default)
                        1 -> SATA0
                lane d: 0 -> slot2 (Default)
                        1 -> SATA1

Using the Device Tree Source File
=================================
To create the DTB (Device Tree Binary) image file, use a command
similar to this::

  dtc -O dtb -b 0 -p 1024 p2041rdb.dts > p2041rdb.dtb

Or use the following command::

  {linux-2.6}/make p2041rdb.dtb ARCH=powerpc

then the dtb file will be generated under the following directory::

  {linux-2.6}/arch/powerpc/boot/p2041rdb.dtb

Booting Linux
=============

Place a linux uImage in the TFTP disk area::

 => tftp 1000000 uImage
 => tftp 2000000 rootfs.ext2.gz.uboot
 => tftp 3000000 p2041rdb.dtb
 => bootm 1000000 2000000 3000000
