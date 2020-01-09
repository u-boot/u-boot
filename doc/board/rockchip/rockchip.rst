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

* rk3288
     - Evb-RK3288
     - Firefly-RK3288
     - mqmaker MiQi
     - Phytec RK3288 PCM-947
     - PopMetal-RK3288
     - Radxa Rock 2 Square
     - Tinker-RK3288
     - Google Jerry
     - Google Mickey
     - Google Minnie
     - Google Speedy
     - Amarula Vyasa-RK3288
* rk3328
     - Rockchip RK3328 EVB
     - Pine64 Rock64
* rk3368
     - GeekBox
     - PX5 EVB
     - Rockchip sheep board
     - Theobroma Systems RK3368-uQ7 SoM
* rk3399
     - 96boards RK3399 Ficus
     - 96boards Rock960
     - Firefly-RK3399 Board
     - Firefly ROC-RK3399-PC Board
     - FriendlyElec NanoPC-T4
     - FriendlyElec NanoPi M4
     - FriendlyARM NanoPi NEO4
     - Google Bob
     - Khadas Edge
     - Khadas Edge-Captain
     - Khadas Edge-V
     - Orange Pi RK3399 Board
     - Pine64 RockPro64
     - Radxa ROCK Pi 4
     - Rockchip RK3399 Evaluation Board
     - Theobroma Systems RK3399-Q7 SoM

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

SD Card
^^^^^^^

All rockchip platforms, except rk3128 (which doesn't use SPL) are now
supporting single boot image using binman and pad_cat.

To write an image that boots from an SD card (assumed to be /dev/sda)::

        sudo dd if=u-boot-rockchip.bin of=/dev/sda seek=64
        sync

TODO
----

- Add rockchip idbloader image building
- Add rockchip TPL image building
- Document SPI flash boot
- Describe steps for eMMC flashing
- Add missing SoC's with it boards list

.. Jagan Teki <jagan@amarulasolutions.com>
.. Fri Jan 10 00:08:40 IST 2020
