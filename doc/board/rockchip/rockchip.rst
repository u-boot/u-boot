.. SPDX-License-Identifier: GPL-2.0+
.. Copyright (C) 2019 Jagan Teki <jagan@amarulasolutions.com>

ROCKCHIP
========

About this
----------

This document describes the information about Rockchip supported boards
and it's usage steps.

Rockchip boards
---------------

Rockchip is SoC solutions provider for tablets & PCs, streaming media
TV boxes, AI audio & vision, IoT hardware.

A wide range of Rockchip SoCs with associated boardsare supported in
mainline U-Boot.

List of mainline supported rockchip boards:

* rk3036
     - Rockchip Evb-RK3036 (evb-rk3036)
     - Kylin (kylin_rk3036)
* rk3128
     - Rockchip Evb-RK3128 (evb-rk3128)
* rk3229
     - Rockchip Evb-RK3229 (evb-rk3229)
* rk3288
     - Rockchip Evb-RK3288 (evb-rk3288)
     - Firefly-RK3288 (firefly-rk3288)
     - MQmaker MiQi (miqi-rk3288)
     - Phytec RK3288 PCM-947 (phycore-rk3288)
     - PopMetal-RK3288 (popmetal-rk3288)
     - Radxa Rock 2 Square (rock2)
     - Tinker-RK3288 (tinker-rk3288)
     - Google Jerry (chromebook_jerry)
     - Google Mickey (chromebook_mickey)
     - Google Minnie (chromebook_minnie)
     - Google Speedy (chromebook_speedy)
     - Amarula Vyasa-RK3288 (vyasa-rk3288)
* rk3308
     - Rockchip Evb-RK3308 (evb-rk3308)
     - Roc-cc-RK3308 (roc-cc-rk3308)
* rk3328
     - Rockchip Evb-RK3328 (evb-rk3328)
     - Pine64 Rock64 (rock64-rk3328)
     - Firefly-RK3328 (roc-cc-rk3328)
     - Radxa Rockpi E (rock-pi-e-rk3328)
* rk3368
     - GeekBox (geekbox)
     - PX5 EVB (evb-px5)
     - Rockchip Sheep (sheep-rk3368)
     - Theobroma Systems RK3368-uQ7 SoM - Lion (lion-rk3368)
* rk3399
     - 96boards RK3399 Ficus (ficus-rk3399)
     - 96boards Rock960 (rock960-rk3399)
     - Firefly-RK3399 (firefly_rk3399)
     - Firefly ROC-RK3399-PC
     - FriendlyElec NanoPC-T4 (nanopc-t4-rk3399)
     - FriendlyElec NanoPi M4 (nanopi-m4-rk3399)
     - FriendlyARM NanoPi NEO4 (nanopi-neo4-rk3399)
     - Google Bob (chromebook_bob)
     - Khadas Edge (khadas-edge-rk3399)
     - Khadas Edge-Captain (khadas-edge-captain-rk3399)
     - Khadas Edge-V (hadas-edge-v-rk3399)
     - Orange Pi RK3399 (orangepi-rk3399)
     - Pine64 RockPro64 (rockpro64-rk3399)
     - Radxa ROCK Pi 4 (rock-pi-4-rk3399)
     - Rockchip Evb-RK3399 (evb_rk3399)
     - Theobroma Systems RK3399-Q7 SoM - Puma (puma_rk3399)
* rv1108
     - Rockchip Evb-rv1108 (evb-rv1108)
     - Elgin-R1 (elgin-rv1108)
* rv3188
     - Radxa Rock (rock)

Building
--------

TF-A
^^^^

TF-A would require to build for ARM64 Rockchip SoCs platforms.

To build TF-A::

        git clone https://github.com/ARM-software/arm-trusted-firmware.git
        cd arm-trusted-firmware
        make realclean
        make CROSS_COMPILE=aarch64-linux-gnu- PLAT=rk3399

Specify the PLAT= with desired rockchip platform to build TF-A for.

U-Boot
^^^^^^

To build rk3328 boards::

        export BL31=/path/to/arm-trusted-firmware/to/bl31.elf
        make evb-rk3328_defconfig
        make

To build rk3288 boards::

        make evb-rk3288_defconfig
        make

To build rk3368 boards::

        export BL31=/path/to/arm-trusted-firmware/to/bl31.elf
        make evb-px5_defconfig
        make

To build rk3399 boards::

        export BL31=/path/to/arm-trusted-firmware/to/bl31.elf
        make evb-rk3399_defconfig
        make

Flashing
--------

1. Package the image with U-Boot TPL/SPL
-----------------------------------------

SD Card
^^^^^^^

All rockchip platforms, except rk3128 (which doesn't use SPL) are now
supporting single boot image using binman and pad_cat.

To write an image that boots from an SD card (assumed to be /dev/sda)::

        sudo dd if=u-boot-rockchip.bin of=/dev/sda seek=64
        sync

eMMC
^^^^

eMMC flash would probe on mmc0 in most of the rockchip platforms.

Create GPT partition layout as defined in configurations::

        mmc dev 0
        gpt write mmc 0 $partitions

Connect the USB-OTG cable between host and target device.

Launch fastboot at target::

        fastboot 0

Upon successful gadget connection,host show the USB device like::

        lsusb
        Bus 001 Device 020: ID 2207:330c Fuzhou Rockchip Electronics Company RK3399 in Mask ROM mode

Program the flash::

        sudo fastboot -i 0x2207 flash loader1 idbloader.img
        sudo fastboot -i 0x2207 flash loader2 u-boot.itb

Note: for rockchip 32-bit platforms the U-Boot proper image
is u-boot-dtb.img

SPI
^^^

Generating idbloader for SPI boot would require to input a multi image
image format to mkimage tool instead of concerting (like for MMC boot).

SPL-alone SPI boot image::

        ./tools/mkimage -n rk3399 -T rkspi -d spl/u-boot-spl.bin idbloader.img

TPL+SPL SPI boot image::

        ./tools/mkimage -n rk3399 -T rkspi -d tpl/u-boot-tpl.bin:spl/u-boot-spl.bin idbloader.img

Copy SPI boot images into SD card and boot from SD::

        sf probe
        load mmc 1:1 $kernel_addr_r idbloader.img
        sf erase 0 +$filesize
        sf write $kernel_addr_r 0 ${filesize}
        load mmc 1:1 ${kernel_addr_r} u-boot.itb
        sf erase 0x60000 +$filesize
        sf write $kernel_addr_r 0x60000 ${filesize}

2. Package the image with Rockchip miniloader
---------------------------------------------

Image package with Rockchip miniloader requires robin [1].

Create idbloader.img

.. code-block:: none

  cd u-boot
  ./tools/mkimage -n px30 -T rksd -d rkbin/bin/rk33/px30_ddr_333MHz_v1.15.bin idbloader.img
  cat rkbin/bin/rk33/px30_miniloader_v1.22.bin >> idbloader.img
  sudo dd if=idbloader.img of=/dev/sda seek=64

Create trust.img

.. code-block:: none

  cd rkbin
  ./tools/trust_merger RKTRUST/PX30TRUST.ini
  sudo dd if=trust.img of=/dev/sda seek=24576

Create uboot.img

.. code-block:: none

  rbink/tools/loaderimage --pack --uboot u-boot-dtb.bin uboot.img 0x200000
  sudo dd if=uboot.img of=/dev/sda seek=16384

Note:
1. 0x200000 is load address and it's an optional in some platforms.
2. rkbin binaries are kept on updating, so would recommend to use the latest versions.

TODO
----

- Add rockchip idbloader image building
- Add rockchip TPL image building
- Document SPI flash boot
- Add missing SoC's with it boards list

[1] https://github.com/rockchip-linux/rkbin

.. Jagan Teki <jagan@amarulasolutions.com>
.. Wednesday 28 October 2020 06:47:26 PM IST
