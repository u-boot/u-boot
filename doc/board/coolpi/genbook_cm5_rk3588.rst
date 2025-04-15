.. SPDX-License-Identifier: GPL-2.0+

GenBook
=======
Cool Pi GenBook is a laptop powered by RK3588, it works with a
carrier board connect with CM5.

Specification:

* Rockchip RK3588
* LPDDR5X 8/32 GB
* eMMC 64 GB
* SPI Nor 8 MB
* HDMI Type A out x 1
* USB 3.0 Host x 1
* USB-C 3.0 with DisplayPort AltMode
* PCIE M.2 E Key for RTL8852BE Wireless connection
* PCIE M.2 M Key for NVME connection
* eDP panel with 1920x1080

Here is the step-by-step to compile and boot to U-Boot on GenBook.

Get the TF-A and DDR init (TPL) binaries
----------------------------------------

.. prompt:: bash

   cd u-boot
   export ROCKCHIP_TPL=../rkbin/bin/rk35/rk3588_ddr_lp4_2112MHz_lp5_2400MHz_v1.17.bin
   export BL31=../rkbin/bin/rk35/rk3588_bl31_v1.46.elf
   make coolpi-cm5-genbook-rk3588_defconfig
   make CROSS_COMPILE=aarch64-linux-gnu-

This will build ``u-boot-rockchip.bin`` for eMMC and ``u-boot-rockchip-spi.bin`` for SPI Nor.

Write u-boot to eMMC or SPI Nor from a Linux system on the laptop
-----------------------------------------------------------------

Copy ``u-boot-rockchip.bin`` and ``u-boot-rockchip-spi.bin`` to the laptop.

eMMC
~~~~

.. prompt:: bash

   dd if=u-boot-rockchip.bin of=/dev/mmcblk0 bs=512 seek=64

SPI Nor
~~~~~~~

.. prompt:: bash

  dd if=u-boot-rockchip-spi.bin of=/dev/mtdblock0

``upgrade_tool`` allows to flash the on-board SPI Nor via the USB TypeC interface
with help of the Rockchip loader binary.

To enter the USB flashing mode, connect the laptop and your HOST PC with a USB-C
cable, reset the laptop with ``Loader Key`` pressed.
On your PC, check with ``lsusb -d 2207:350b``).

To flash U-Boot on the SPI Nor with ``upgrade_tool``:

.. prompt:: bash

  upgrade_tool db rk3588/MiniLoaderAll.bin
  upgrade_tool ssd       // Input 5 for SPINOR download mode
  upgrade_tool wl 0 u-boot-rockchip-spi.bin
  upgrade_tool rd
