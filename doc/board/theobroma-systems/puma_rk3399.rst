.. SPDX-License-Identifier: GPL-2.0+

RK3399-Q7 Puma
==============

The RK3399-Q7 (Puma) is a system-on-module featuring the Rockchip
RK3399 in a Qseven-compatible form-factor.

RK3399-Q7 features:

 * CPU: ARMv8 64bit Big-Little architecture,

   * Big: dual-core Cortex-A72
   * Little: quad-core Cortex-A53
   * IRAM: 200KB
   * DRAM: 4GB-128MB dual-channel

 * eMMC: onboard eMMC
 * SD/MMC
 * GbE (onboard Micrel KSZ9031) Gigabit ethernet PHY
 * USB:

   * USB3.0 dual role port
   * 2x USB3.0 host, 1x USB2.0 host via onboard USB3.0 hub

 * Display: HDMI/eDP/MIPI
 * Camera: 2x CSI (one on the edge connector, one on the Q7 specified CSI ZIF)
 * NOR Flash: onboard SPI NOR
 * Companion Controller: onboard additional Cortex-M0 microcontroller

   * RTC
   * fan controller
   * CAN

Here is the step-by-step to boot to U-Boot on RK3399-Q7 from Theobroma Systems.

Get the Source and build ATF binary
-----------------------------------

.. prompt:: bash

   git clone https://git.trustedfirmware.org/TF-A/trusted-firmware-a.git
   cd trusted-firmware-a
   make CROSS_COMPILE=aarch64-linux-gnu- PLAT=rk3399 bl31
   export BL31=$PWD/build/rk3399/release/bl31/bl31.elf

Compile the U-Boot
------------------

.. prompt:: bash

   cd ../u-boot
   make CROSS_COMPILE=aarch64-linux-gnu- puma-rk3399_defconfig all

This will build ``u-boot-rockchip.bin`` which can be written to an MMC device
(eMMC or SD card), and ``u-boot-rockchip-spi.bin`` which can be written to the
SPI-NOR flash.

Flash the image
---------------

Copy ``u-boot-rockchip.bin`` to offset 32k for SD/eMMC.
Copy ``u-boot-rockchip-spi.bin`` to offset 0 for NOR-flash.

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
(check with ``lsusb -d 2207:330c``).

To flash U-Boot on the eMMC with ``rkdeveloptool``:

.. prompt:: bash

   git clone https://github.com/rockchip-linux/rkdeveloptool
   cd rkdeveloptool
   autoreconf -i && CPPFLAGS=-Wno-format-truncation ./configure && make
   git clone https://github.com/rockchip-linux/rkbin.git
   cd rkbin
   ./tools/boot_merger RKBOOT/RK3399MINIALL.ini
   cd ..
   ./rkdeveloptool db rkbin/rk3399_loader_v1.30.130.bin
   ./rkdeveloptool wl 64 ../u-boot-rockchip.bin

NOR-Flash
~~~~~~~~~

``rkdeveloptool`` allows to flash the on-board SPI via the USB OTG interface with
help of the Rockchip loader binary.

To enter the USB flashing mode on Haikou baseboard, remove any SD card, insert a
micro-USB cable in the ``Q7 USB P1`` connector (P8), move ``SW5`` switch into
``BIOS Disable`` mode, power cycle or reset the board and move ``SW5`` switch
back to ``Normal Boot`` mode. A new USB device should have appeared on your PC
(check with ``lsusb -d 2207:330c``).

To flash U-Boot on the SPI with ``rkdeveloptool``:

.. prompt:: bash

   git clone https://github.com/rockchip-linux/rkdeveloptool
   cd rkdeveloptool
   autoreconf -i && CPPFLAGS=-Wno-format-truncation ./configure && make
   git clone https://github.com/rockchip-linux/rkbin.git
   cd rkbin
   ./tools/boot_merger RKBOOT/RK3399MINIALL_SPINOR.ini
   cd ..
   ./rkdeveloptool db rkbin/rk3399_loader_spinor_v1.30.114.bin
   ./rkdeveloptool ef
   ./rkdeveloptool wl 0 ../u-boot-rockchip-spi.bin
