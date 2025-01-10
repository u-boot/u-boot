.. SPDX-License-Identifier: GPL-2.0+

SBC-RK3588-AMR Jaguar
=====================

The SBC-RK3588-AMR is a Single Board Computer designed by
Theobroma Systems for autonomous mobile robots.

It provides the following features:

 * up to 32GB LDDR4
 * up to 128GB on-module eMMC (with 8-bit 1.8V interface)
 * SD card
 * Gigabit Ethernet
 * 1x USB-A 2.0 host
 * PCIe M.2 2230 Key M (Gen 2 1-lane) for WiFi+BT
 * PCIe M.2 2280 Key M (Gen 3 4-lane) for NVMe
 * CAN
 * RS485 UART
 * 2x USB Type-C 3.1 host/device
 * HDMI output
 * 2x camera connectors (MIPI-CSI 2-lane I2C/SPI for IMUs GPIOs)
 * EEPROM
 * Secure Element
 * ATtiny companion controller implementing:

   - low-power RTC functionality (ISL1208 emulation)
   - fan controller (AMC6821 emulation)

 * 80-pin Mezzanine connector

Here is the step-by-step to boot to U-Boot on SBC-RK3588-AMR Jaguar from Theobroma
Systems.

Get the TF-A and DDR init (TPL) binaries
----------------------------------------

.. prompt:: bash

   git clone https://github.com/rockchip-linux/rkbin
   cd rkbin
   export RKBIN=$(pwd)
   export BL31=$RKBIN/bin/rk35/rk3588_bl31_v1.47.elf
   export ROCKCHIP_TPL=$RKBIN/bin/rk35/rk3588_ddr_lp4_2112MHz_lp5_2400MHz_v1.18.bin
   sed -i 's/^uart baudrate=.*$/uart baudrate=115200/' tools/ddrbin_param.txt
   ./tools/ddrbin_tool rk3588 tools/ddrbin_param.txt "$ROCKCHIP_TPL"
   ./tools/boot_merger RKBOOT/RK3588MINIALL.ini
   export RKDB=$RKBIN/rk3588_spl_loader_v1.11.112.bin

This will setup all required external dependencies for compiling U-Boot. This will
be updated in the future once upstream Trusted-Firmware-A supports RK3588 or U-Boot
gains support for open-source DRAM initialization in TPL.

Build U-Boot
------------

.. prompt:: bash

   cd ../u-boot
   make CROSS_COMPILE=aarch64-linux-gnu- jaguar-rk3588_defconfig all

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

To enter the USB flashing mode, remove any SD card, insert a USB-C cable in the
``DOWNLOAD`` USB Type-C connector (P11) and then power cycle or reset the board
while pressing the ``BIOS`` (SW2) button. A new USB device should have appeared
on your PC (check with ``lsusb -d 2207:350b``).

To flash U-Boot on the eMMC with ``rkdeveloptool``:

.. prompt:: bash

   git clone https://github.com/rockchip-linux/rkdeveloptool
   cd rkdeveloptool
   autoreconf -i && CPPFLAGS=-Wno-format-truncation ./configure && make
   ./rkdeveloptool db "$RKDB"
   ./rkdeveloptool wl 64 ../u-boot-rockchip.bin
