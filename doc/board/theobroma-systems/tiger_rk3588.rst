.. SPDX-License-Identifier: GPL-2.0+

SOM-RK3588-Q7 Tiger
===================

The RK3588-Q7 SoM is a Qseven-compatible (70mm x 70mm, MXM-230
connector) system-on-module from Theobroma Systems, featuring the
Rockchip RK3588.

It provides the following feature set:

 * up to 16GB LPDDR4x
 * on-module eMMC
 * SD card (on a baseboard) via edge connector
 * Gigabit Ethernet with on-module GbE PHY
 * HDMI/eDP
 * MIPI-DSI
 * 4x MIPI-CSI (3x on FPC connectors, 1x over Q7)
 * HDMI input over FPC connector
 * CAN
 * USB

   - 1x USB 3.0 dual-role (direct connection)
   - 2x USB 3.0 host + 1x USB 2.0 host

 * PCIe

   - 1x PCIe 2.1 Gen3, 4 lanes
   - 2xSATA / 2x PCIe 2.1 Gen1, 2 lanes

 * on-module ATtiny816 companion controller, implementing:

   - low-power RTC functionality (ISL1208 emulation)
   - fan controller (AMC6821 emulation)

 * on-module Secure Element with Global Platform 2.2.1 compliant
   JavaCard environment

Here is the step-by-step to boot to U-Boot on SOM-RK3588-Q7 Tiger from Theobroma
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
   sed -i 's/^uart iomux=.*$/uart iomux=2/' tools/ddrbin_param.txt
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
   make CROSS_COMPILE=aarch64-linux-gnu- tiger-rk3588_defconfig all

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
(check with ``lsusb -d 2207:350b``).

To flash U-Boot on the eMMC with ``rkdeveloptool``:

.. prompt:: bash

   git clone https://github.com/rockchip-linux/rkdeveloptool
   cd rkdeveloptool
   autoreconf -i && CPPFLAGS=-Wno-format-truncation ./configure && make
   ./rkdeveloptool db "$RKDB"
   ./rkdeveloptool wl 64 ../u-boot-rockchip.bin
