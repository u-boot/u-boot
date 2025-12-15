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

Get DDR init (TPL) binary
-------------------------

.. prompt:: bash

   git clone https://github.com/rockchip-linux/rkbin
   cd rkbin
   export ROCKCHIP_TPL=$(readlink -f bin/rk35/rk3588_ddr_lp4_2112MHz_lp5_2400MHz_v*.bin | head -1)
   sed -i 's/^uart baudrate=.*$/uart baudrate=115200/' tools/ddrbin_param.txt
   sed -i 's/^uart iomux=.*$/uart iomux=0/' tools/ddrbin_param.txt
   python3 ./tools/ddrbin_tool.py rk3588 tools/ddrbin_param.txt "$ROCKCHIP_TPL"
   ./tools/boot_merger RKBOOT/RK3588MINIALL.ini
   export RKDB=$(readlink -f rk3588_spl_loader_v*.bin | head -1)

This will setup all required external dependencies for compiling U-Boot. This will
be updated in the future once U-Boot gains support for open-source DRAM initialization
in TPL.

Get TF-A
--------

There are two possible options, pick one or the other. Note that the instructions need
to be run from the ``rkbin`` directory.

Prebuilt binary from Rockchip
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

.. prompt:: bash

   export BL31=$(readlink -f bin/rk35/rk3588_bl31_v*.elf | head -1)

Upstream
~~~~~~~~

.. prompt:: bash

   cd ../
   git clone https://git.trustedfirmware.org/TF-A/trusted-firmware-a.git
   cd trusted-firmware-a
   make CROSS_COMPILE=aarch64-linux-gnu- PLAT=rk3588 bl31
   export BL31=$PWD/build/rk3588/release/bl31/bl31.elf

Build U-Boot
------------

.. prompt:: bash

   cd ../u-boot
   make CROSS_COMPILE=aarch64-linux-gnu- jaguar-rk3588_defconfig all

.. note::
   If using upstream TF-A, one should disable ``SPL_ATF_NO_PLATFORM_PARAM`` symbol in
   U-Boot config (via e.g. ``make CROSS_COMPILE=aarch64-linux-gnu- menuconfig``) which
   will, among other things, enable console output in TF-A.

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
