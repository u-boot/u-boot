.. SPDX-License-Identifier: GPL-2.0-or-later

HiKey board
###########

Introduction
============

HiKey is the first certified 96Boards Consumer Edition board. The board/SoC has:

* HiSilicon Kirin 6220 eight-core ARM Cortex-A53 64-bit SoC running at 1.2GHz.
* ARM Mali 450-MP4 GPU
* 1GB 800MHz LPDDR3 DRAM
* 4GB eMMC Flash Storage
* microSD
* 802.11a/b/g/n WiFi, Bluetooth

The HiKey schematic can be found here:
https://github.com/96boards/documentation/blob/master/consumer/hikey/hikey620/hardware-docs/HiKey_schematics_LeMaker_version_Rev_A1.pdf

The SoC datasheet can be found here:
https://github.com/96boards/documentation/blob/master/consumer/hikey/hikey620/hardware-docs/Hi6220V100_Multi-Mode_Application_Processor_Function_Description.pdf

Currently the u-boot port supports:

* USB
* eMMC
* SD card
* GPIO

The HiKey U-Boot port has been tested with l-loader, booting ATF, which then
boots U-Boot as the bl33.bin executable.

Compile from source
===================

First get all the sources

.. code-block:: bash

  mkdir -p ~/hikey/src ~/hikey/bin
  cd ~/hikey/src
  git clone https://github.com/96boards-hikey/edk2 -b testing/hikey960_v2.5
  git clone https://github.com/TrustedFirmware-A/trusted-firmware-a
  git clone https://github.com/96boards-hikey/l-loader -b testing/hikey960_v1.2
  git clone https://github.com/96boards-hikey/OpenPlatformPkg -b testing/hikey960_v1.3.4
  git clone https://github.com/96boards-hikey/atf-fastboot
  wget https://snapshots.linaro.org/96boards/reference-platform/components/uefi-staging/latest/hikey/release/hisi-idt.py

Get the BL30 mcuimage.bin binary. It is shipped as part of the UEFI source.
The latest version can be obtained from the OpenPlatformPkg repo.

.. code-block:: bash

  cp OpenPlatformPkg/Platforms/Hisilicon/HiKey/Binary/mcuimage.bin ~/hikey/bin/

Get nvme.img binary

.. code-block:: bash

  wget -P ~/hikey/bin https://snapshots.linaro.org/96boards/reference-platform/components/uefi-staging/latest/hikey/release/nvme.img

Compile U-Boot
==============

.. code-block:: bash

  cd ~/hikey/src/u-boot
  make CROSS_COMPILE=aarch64-linux-gnu- hikey_config
  make CROSS_COMPILE=aarch64-linux-gnu-
  cp u-boot.bin ~/hikey/bin

Compile ARM Trusted Firmware (ATF)
==================================

.. code-block:: bash

  cd ~/hikey/src/trusted-firmware-a
  make CROSS_COMPILE=aarch64-linux-gnu- all fip \
    SCP_BL2=~/hikey/bin/mcuimage.bin \
    BL33=~/hikey/bin/u-boot.bin DEBUG=1 PLAT=hikey

Copy the resulting FIP binary

.. code-block:: bash

  cp build/hikey/debug/fip.bin ~/hikey/bin

Compile ATF Fastboot
====================

.. code-block:: bash

  cd ~/hikey/src/atf-fastboot
  make CROSS_COMPILE=aarch64-linux-gnu- PLAT=hikey DEBUG=1

Compile l-loader
================

.. code-block:: bash

  cd ~/hikey/src/l-loader
  ln -sf ~/hikey/src/trusted-firmware-a/build/hikey/debug/bl1.bin
  ln -sf ~/hikey/src/trusted-firmware-a/build/hikey/debug/bl2.bin
  ln -sf ~/hikey/src/atf-fastboot/build/hikey/debug/bl1.bin fastboot.bin
  make hikey PTABLE_LST=aosp-8g

Copy the resulting binaries

.. code-block:: bash

  cp *.img ~/hikey/bin
  cp l-loader.bin ~/hikey/bin
  cp recovery.bin ~/hikey/bin

These instructions are adapted from
https://github.com/TrustedFirmware-A/trusted-firmware-a/blob/master/docs/plat/hikey.rst

Flashing
========

1. Connect the second jumper on J15 BOOT SEL, to go into recovery mode and flash l-loader.bin with
the hisi-idt.py utility. Then connect a USB A to B mini cable from your PC to the USB OTG port of HiKey and execute the below command.

The command below assumes HiKey enumerated as the first USB serial port

.. code-block:: bash

  sudo python ~/hikey/src/hisi-idt.py -d /dev/ttyUSB0 --img1 ~/hikey/bin/recovery.bin

2. Once LED 0 comes on solid, HiKey board should be detected as a fastboot device.

.. code-block::

  sudo fastboot devices

  0123456789ABCDEF	fastboot

3. Flash the images

.. code-block::

  sudo fastboot flash ptable ~/hikey/bin/prm_ptable.img
  sudo fastboot flash loader ~/hikey/bin/l-loader.bin
  sudo fastboot flash fastboot ~/hikey/bin/fip.bin
  sudo fastboot flash nvme ~/hikey/bin/nvme.img

4. Disconnect second jumper on J15 BOOT SEL, and reset the board and you will now (hopefully)
   have ATF, booting u-boot from eMMC.

   Note: To get USB host working, also disconnect the USB OTG cable used for flashing. Otherwise you
   will get 'dwc_otg_core_host_init: Timeout!' errors.

See working boot trace below on UART3 available at Low Speed Expansion header::

  NOTICE:  BL2: v1.5(debug):v1.5-694-g6d4f6aea
  NOTICE:  BL2: Built : 09:21:42, Aug 29 2018
  INFO:    BL2: Doing platform setup
  INFO:    ddr3 rank1 init pass
  INFO:    succeed to set ddrc 150mhz
  INFO:    ddr3 rank1 init pass
  INFO:    succeed to set ddrc 266mhz
  INFO:    ddr3 rank1 init pass
  INFO:    succeed to set ddrc 400mhz
  INFO:    ddr3 rank1 init pass
  INFO:    succeed to set ddrc 533mhz
  INFO:    ddr3 rank1 init pass
  INFO:    succeed to set ddrc 800mhz
  INFO:    Samsung DDR
  INFO:    ddr test value:0xa5a55a5a
  INFO:    BL2: TrustZone: protecting 16777216 bytes of memory at 0x3f000000
  INFO:    BL2: TrustZone: protecting 4194304 bytes of memory at 0x3e800000
  INFO:    [BDID] [fff91c18] midr: 0x410fd033
  INFO:    init_acpu_dvfs: pmic version 17
  INFO:    init_acpu_dvfs: ACPU_CHIP_MAX_FREQ=0x186a00.
  INFO:    acpu_dvfs_volt_init: success!
  INFO:    acpu_dvfs_set_freq: support freq num is 5
  INFO:    acpu_dvfs_set_freq: start prof is 0x4
  INFO:    acpu_dvfs_set_freq: magic is 0x5a5ac5c5
  INFO:    acpu_dvfs_set_freq: voltage:
  INFO:      - 0: 0x49
  INFO:      - 1: 0x49
  INFO:      - 2: 0x50
  INFO:      - 3: 0x60
  INFO:      - 4: 0x78
  NOTICE:  acpu_dvfs_set_freq: set acpu freq success!INFO:    BL2: Loading image id 2
  INFO:    Loading image id=2 at address 0x1000000
  INFO:    Image id=2 loaded: 0x1000000 - 0x1023d00
  INFO:    hisi_mcu_load_image: mcu sections 0:
  INFO:    hisi_mcu_load_image:  src  = 0x1000200
  INFO:    hisi_mcu_load_image:  dst  = 0xf6000000
  INFO:    hisi_mcu_load_image:  size = 31184
  INFO:    hisi_mcu_load_image:  [SRC 0x1000200] 0x8000 0x3701 0x7695 0x7689
  INFO:    hisi_mcu_load_image:  [DST 0xf6000000] 0x8000 0x3701 0x7695 0x7689
  INFO:    hisi_mcu_load_image: mcu sections 1:
  INFO:    hisi_mcu_load_image:  src  = 0x1007bd0
  INFO:    hisi_mcu_load_image:  dst  = 0x5e00000
  INFO:    hisi_mcu_load_image:  size = 93828
  INFO:    hisi_mcu_load_image:  [SRC 0x1007bd0] 0xf000b510 0x2103fb3d 0xf0004604 0xf003fb57
  INFO:    hisi_mcu_load_image:  [DST 0x5e00000] 0xf000b510 0x2103fb3d 0xf0004604 0xf003fb57
  INFO:    hisi_mcu_load_image: mcu sections 2:
  INFO:    hisi_mcu_load_image:  src  = 0x101ea54
  INFO:    hisi_mcu_load_image:  dst  = 0x5e16e84
  INFO:    hisi_mcu_load_image:  size = 15428
  INFO:    hisi_mcu_load_image:  [SRC 0x101ea54] 0x9 0x1020640 0x10001 0x8f0d180
  INFO:    hisi_mcu_load_image:  [DST 0x5e16e84] 0x9 0x1020640 0x10001 0x8f0d180
  INFO:    hisi_mcu_load_image: mcu sections 3:
  INFO:    hisi_mcu_load_image:  src  = 0x1022698
  INFO:    hisi_mcu_load_image:  dst  = 0x5e22a10
  INFO:    hisi_mcu_load_image:  size = 3060
  INFO:    hisi_mcu_load_image:  [SRC 0x1022698] 0x0 0x0 0x0 0x0
  INFO:    hisi_mcu_load_image:  [DST 0x5e22a10] 0x0 0x0 0x0 0x0
  INFO:    hisi_mcu_load_image: mcu sections 4:
  INFO:    hisi_mcu_load_image:  src  = 0x102328c
  INFO:    hisi_mcu_load_image:  dst  = 0x5e23604
  INFO:    hisi_mcu_load_image:  size = 2616
  INFO:    hisi_mcu_load_image:  [SRC 0x102328c] 0xf80000a0 0x0 0xf80000ac 0x0
  INFO:    hisi_mcu_load_image:  [DST 0x5e23604] 0xf80000a0 0x0 0xf80000ac 0x0
  INFO:    hisi_mcu_start_run: AO_SC_SYS_CTRL2=0
  INFO:    plat_hikey_bl2_handle_scp_bl2: MCU PC is at 0x42933301
  INFO:    plat_hikey_bl2_handle_scp_bl2: AO_SC_PERIPH_CLKSTAT4 is 0x3b018f09
  WARNING: BL2: Platform setup already done!!
  INFO:    BL2: Loading image id 3
  INFO:    Loading image id=3 at address 0xf9858000
  INFO:    Image id=3 loaded: 0xf9858000 - 0xf9860058
  INFO:    BL2: Loading image id 5
  INFO:    Loading image id=5 at address 0x35000000
  INFO:    Image id=5 loaded: 0x35000000 - 0x35061cd2
  NOTICE:  BL2: Booting BL31
  INFO:    Entry point address = 0xf9858000
  INFO:    SPSR = 0x3cd
  NOTICE:  BL31: v1.5(debug):v1.5-694-g6d4f6aea
  NOTICE:  BL31: Built : 09:21:44, Aug 29 2018
  WARNING: Using deprecated integer interrupt array in gicv2_driver_data_t
  WARNING: Please migrate to using an interrupt_prop_t array
  INFO:    ARM GICv2 driver initialized
  INFO:    BL31: Initializing runtime services
  INFO:    BL31: cortex_a53: CPU workaround for disable_non_temporal_hint was applied
  INFO:    BL31: cortex_a53: CPU workaround for 843419 was applied
  INFO:    BL31: cortex_a53: CPU workaround for 855873 was applied
  INFO:    BL31: Preparing for EL3 exit to normal world
  INFO:    Entry point address = 0x35000000
  INFO:    SPSR = 0x3c9

  U-Boot 2018.09-rc1 (Aug 22 2018 - 14:55:49 +0530)hikey

  DRAM:  990 MiB
  HI6553 PMIC init
  MMC:   config_sd_carddetect: SD card present
  Hisilicon DWMMC: 0, Hisilicon DWMMC: 1
  Loading Environment from FAT... Unable to use mmc 1:1... Failed (-5)
  In:    uart@f7113000
  Out:   uart@f7113000
  Err:   uart@f7113000
  Net:   Net Initialization Skipped
  No ethernet found.
  Hit any key to stop autoboot:  0
  starting USB...
  USB0:   scanning bus 0 for devices... 2 USB Device(s) found
         scanning usb for storage devices... 0 Storage Device(s) found
         scanning usb for ethernet devices... 0 Ethernet Device(s) found
