.. SPDX-License-Identifier: GPL-2.0+

PX30-uQ7 Ringneck
=================

The PX30-uQ7 (Ringneck) SoM is a µQseven-compatible (40mmx70mm, MXM-230
connector) system-on-module from Theobroma Systems, featuring the Rockchip PX30.

It provides the following feature set:

  * up to 4GB DDR4
  * up to 128GB on-module eMMC (with 8-bit 1.8V interface)
  * SD card (on a baseboard) via edge connector
  * Fast Ethernet with on-module TI DP83825I PHY
  * MIPI-DSI/LVDS
  * MIPI-CSI
  * USB

    - 1x USB 2.0 dual-role
    - 3x USB 2.0 host

  * on-module companion controller (STM32 Cortex-M0 or ATtiny), implementing:

    - low-power RTC functionality (ISL1208 emulation)
    - fan controller (AMC6821 emulation)
    - USB<->CAN bridge controller (STM32 only)

  * on-module Espressif ESP32 for Bluetooth + 2.4GHz WiFi
  * on-module NXP SE05x Secure Element

Here is the step-by-step to boot to U-Boot on PX30-uQ7 Ringneck from Theobroma
Systems.

Get the Source and build ATF binary
-----------------------------------

.. prompt:: bash

   git clone https://git.trustedfirmware.org/TF-A/trusted-firmware-a.git
   cd trusted-firmware-a
   make CROSS_COMPILE=aarch64-linux-gnu- PLAT=px30 bl31
   export BL31=$PWD/build/px30/release/bl31/bl31.elf

Compile the U-Boot
------------------

.. prompt:: bash

   cd ../u-boot
   make CROSS_COMPILE=aarch64-linux-gnu- ringneck-px30_defconfig all

This will build ``u-boot-rockchip.bin`` which can be written to an MMC device
(eMMC or SD card).

Flash the image
---------------

Copy ``u-boot-rockchip.bin`` to offset 32k for SD/eMMC.

SD-Card
~~~~~~~

.. prompt:: bash

   dd if=u-boot-rockchip.bin of=/dev/sdX seek=64

.. note::

   Replace ``/dev/sdX`` to match your SD card kernel device.

eMMC
~~~~

``rkdeveloptool`` allows to flash the on-board eMMC via the USB OTG interface
with help of the Rockchip loader binary.

To enter the USB flashing mode on Haikou baseboard, remove any SD card, insert a
micro-USB cable in the ``Q7 USB P1`` connector (P8), move ``SW5`` switch into
``BIOS Disable`` mode, power cycle or reset the board and move ``SW5`` switch
back to ``Normal Boot`` mode. A new USB device should have appeared on your PC
(check with ``lsusb -d 2207:330d``).

To flash U-Boot on the eMMC with ``rkdeveloptool``:

.. prompt:: bash

   git clone https://github.com/rockchip-linux/rkdeveloptool
   cd rkdeveloptool
   autoreconf -i && CPPFLAGS=-Wno-format-truncation ./configure && make
   git clone https://github.com/rockchip-linux/rkbin.git
   cd rkbin
   ./tools/boot_merger RKBOOT/PX30MINIALL.ini
   cd ..
   ./rkdeveloptool db rkbin/px30_loader_v2.08.135.bin
   ./rkdeveloptool wl 64 ../u-boot-rockchip.bin
