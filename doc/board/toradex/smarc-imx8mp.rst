.. SPDX-License-Identifier: GPL-2.0-or-later

Toradex SMARC iMX8M Plus Module
===============================

- SoM: https://www.toradex.com/computer-on-modules/smarc-arm-family/nxp-imx-8m-plus
- Carrier board: https://www.toradex.com/products/carrier-board/smarc-development-board-kit

Quick Start
-----------

- Build the ARM trusted firmware binary
- Get the DDR firmware
- Build U-Boot
- Flash to eMMC
- Boot

Get and Build the ARM Trusted Firmware (Trusted Firmware A)
-----------------------------------------------------------

.. code-block:: bash

    $ echo "Downloading and building TF-A..."
    $ git clone https://git.trustedfirmware.org/TF-A/trusted-firmware-a.git
    $ cd trusted-firmware-a

Then build ATF (TF-A):

.. code-block:: bash

    $ export CROSS_COMPILE=aarch64-linux-gnu-
    $ make PLAT=imx8mp IMX_BOOT_UART_BASE=0x30a60000 bl31
    $ cp build/imx8mp/release/bl31.bin ../

Get the DDR Firmware
--------------------

.. code-block:: bash

    $ cd ..
    $ wget https://www.nxp.com/lgfiles/NMG/MAD/YOCTO/firmware-imx-8.10.1.bin
    $ chmod +x firmware-imx-8.10.1.bin
    $ ./firmware-imx-8.10.1.bin
    $ cp firmware-imx-8.10.1/firmware/ddr/synopsys/lpddr4*_202006.bin ./

Build U-Boot
------------

.. code-block:: bash

    $ export CROSS_COMPILE=aarch64-linux-gnu-
    $ make toradex-smarc-imx8mp_defconfig
    $ make

Flash to eMMC
-------------

.. code-block:: bash

    > tftpboot ${loadaddr} flash.bin
    > setexpr blkcnt ${filesize} + 0x1ff && setexpr blkcnt ${blkcnt} / 0x200
    > mmc dev 0 1 && mmc write ${loadaddr} 0x0 ${blkcnt}

As a convenience, instead of the last two commands, one may also use the update
U-Boot wrapper:

.. code-block:: bash

    > run update_uboot

Boot
----

ATF, U-Boot proper and u-boot.dtb images are packed into a FIT image,
which is loaded and parsed by SPL.

Boot sequence is:

* SPL ---> ATF (TF-A) ---> U-Boot proper

Output::

  U-Boot SPL 2025.04-rc5-00023-g34c31be81211 (Apr 04 2025 - 15:36:23 +0100)
  Training FAILED
  DDR configured as single rank
  SEC0:  RNG instantiated
  Normal Boot
  Trying to boot from BOOTROM
  Boot Stage: Primary boot
  Find img info 0x4802f200, size 1100
  Need continue download 1024
  NOTICE:  Do not release JR0 to NS as it can be used by HAB
  NOTICE:  BL31: v2.11.0(release):v2.11.0-723-gbd298f5c30ac
  NOTICE:  BL31: Built : 14:18:43, Apr  4 2025


  U-Boot 2025.04-rc5-00023-g34c31be81211 (Apr 04 2025 - 15:36:23 +0100)

  CPU:   Freescale i.MX8MP[8] rev1.1 1600 MHz (running at 1200 MHz)
  CPU:   Industrial temperature grade (-40C to 105C) at 72C
  Reset cause: POR
  DRAM:  4 GiB
  Core:  312 devices, 32 uclasses, devicetree: separate
  WDT:   Started watchdog@30280000 with servicing every 1000ms (60s timeout)
  MMC:   FSL_SDHC: 1, FSL_SDHC: 0
  Loading Environment from MMC... Reading from MMC(0)... OK
  In:    serial@30a60000
  Out:   serial@30a60000
  Err:   serial@30a60000
  Model: Toradex 0097 SMARC iMX8M Plus Quad 4GB WB IT V1.0A
  Serial#: 15603364
  SEC0:  RNG instantiated
  Net:   Get shared mii bus on ethernet@30be0000
  eth1: ethernet@30be0000, eth0: ethernet@30bf0000 [PRIME]
  Hit any key to stop autoboot:  0
  SMARC iMX8MP #
