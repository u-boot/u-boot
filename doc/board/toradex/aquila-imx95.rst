.. SPDX-License-Identifier: GPL-2.0-or-later

Toradex Aquila iMX95 Module
===========================

- SoM: https://www.toradex.com/computer-on-modules/aquila-arm-family/nxp-imx95
- Carrier board: https://www.toradex.com/products/carrier-board/aquila-development-board-kit

Quick Start
-----------

- Setup environment
- Get ahab-container.img
- Get DDR PHY Firmware Images
- Get and Build OEI Images
- Get and Build System Manager Image
- Get and Build the ARM Trusted Firmware
- Build the Bootloader Image
- Boot

Setup environment
-----------------

Suggested current toolchains are ARM 14.3 (https://developer.arm.com/downloads/-/arm-gnu-toolchain-downloads):

- https://developer.arm.com/-/media/Files/downloads/gnu/14.3.rel1/binrel/arm-gnu-toolchain-14.3.rel1-x86_64-arm-none-linux-gnueabihf.tar.xz
- https://developer.arm.com/-/media/Files/downloads/gnu/14.3.rel1/binrel/arm-gnu-toolchain-14.3.rel1-x86_64-aarch64-none-linux-gnu.tar.xz

.. code-block:: console

    $ export TOOLS=<path/to/directory/with/toolchains>
    $ export CROSS_COMPILE_32=<path/to/arm/toolchain/bin/>arm-none-linux-gnueabihf-
    $ export CROSS_COMPILE_64=<path/to/arm64/toolchain/bin/>aarch64-none-linux-gnu-

Get ahab-container.img
----------------------

Note: `$srctree` is the U-Boot source directory

.. code-block:: console

    $ wget https://www.nxp.com/lgfiles/NMG/MAD/YOCTO/firmware-ele-imx-2.0.2-89161a8.bin
    $ sh firmware-ele-imx-2.0.2-89161a8.bin --auto-accept
    $ cp firmware-ele-imx-2.0.2-89161a8/mx95b0-ahab-container.img $(srctree)

Get DDR PHY Firmware Images
---------------------------

Note: `$srctree` is the U-Boot source directory

.. code-block:: console

    $ wget https://www.nxp.com/lgfiles/NMG/MAD/YOCTO/firmware-imx-8.28-994fa14.bin
    $ sh firmware-imx-8.28-994fa14.bin --auto-accept
    $ cp firmware-imx-8.28-994fa14/firmware/ddr/synopsys/lpddr5*v202409.bin $(srctree)

Get and Build OEI Images
------------------------

Note: `$srctree` is the U-Boot source directory
Get OEI from: https://git.toradex.com/cgit/imx-oei-toradex.git/
branch: main

.. code-block:: console

    $ git clone -b main https://git.toradex.com/cgit/imx-oei-toradex.git/
    $ cd imx-oei-toradex

    $ make board=toradex-aquila-imx95 oei=ddr DEBUG=1 r=B0 all
    $ cp build/toradex-aquila-imx95/ddr/oei-m33-ddr.bin $(srctree)

    $ make board=toradex-aquila-imx95 oei=tcm DEBUG=1 r=B0 all
    $ cp build/toradex-aquila-imx95/tcm/oei-m33-tcm.bin $(srctree)

The Makefile will set `DDR_CONFIG` automatically based on the selected silicon
revision.

Get and Build the System Manager Image
--------------------------------------

Note: `$srctree` is the U-Boot source directory
Get System Manager from: https://git.toradex.com/cgit/imx-sm-toradex.git/
branch: main

.. code-block:: console

    $ git clone -b main https://git.toradex.com/cgit/imx-sm-toradex.git/
    $ cd imx-sm-toradex
    $ make config=aquila-imx95 all
    $ cp build/aquila-imx95/m33_image.bin $(srctree)

Get and Build the ARM Trusted Firmware
--------------------------------------

Note: `$srctree` is the U-Boot source directory
Get ATF from: https://github.com/nxp-imx/imx-atf/
branch: lf_v2.12

.. code-block:: console

    $ export CROSS_COMPILE=$CROSS_COMPILE_64
    $ unset LDFLAGS
    $ unset AS
    $ git clone -b lf_v2.12 https://github.com/nxp-imx/imx-atf.git
    $ cd imx-atf
    $ make PLAT=imx95 bl31
    $ cp build/imx95/release/bl31.bin $(srctree)

Build the Bootloader Image
--------------------------

.. code-block:: console

    $ export CROSS_COMPILE=$CROSS_COMPILE_64
    $ make aquila-imx95_defconfig
    $ make

Flash to eMMC
-------------

.. code-block:: console

    > tftpboot ${loadaddr} flash.bin
    > setexpr blkcnt ${filesize} + 0x1ff && setexpr blkcnt ${blkcnt} / 0x200
    > mmc dev 0 1 && mmc write ${loadaddr} 0x0 ${blkcnt}

As a convenience, instead of the last two commands, one may also use the update
U-Boot wrapper:

.. code-block:: console

    > run update_uboot

Boot
----

Boot sequence is:

* SPL ---> ATF (TF-A) ---> U-Boot proper

Output:

.. code-block:: console

    U-Boot SPL 2026.07-rc3-00300-ge16706b72e14 (Jun 11 2026 - 13:07:49 +0200)
    SYS Boot reason: por, origin: -1, errid: -1
    WDT:   Started watchdog@42490000 with servicing every 1000ms (40s timeout)
    Trying to boot from MMC1
    Load image from MMC/SD 0xd2800
    NOTICE:  BL31: v2.10.0  (release):lf-6.6.52-2.2.1-dirty
    NOTICE:  BL31: Built : 06:40:36, Jul  7 2025


    U-Boot 2026.07-rc3-00300-ge16706b72e14 (Jun 11 2026 - 13:07:49 +0200)

    CPU:   NXP i.MX95 Rev2.0 A55 at 1800 MHz - invalid sensor data
    DRAM:  7.8 GiB
    Core:  321 devices, 30 uclasses, devicetree: separate
    WDT:   Started watchdog@42490000 with servicing every 1000ms (40s timeout)
    MMC:   FSL_SDHC: 0, FSL_SDHC: 1
    Loading Environment from MMC... Reading from MMC(0)... OK
    In:    serial@44380000
    Out:   serial@44380000
    Err:   serial@44380000
    Model: Toradex 0098 Aquila iMX95 Hexa 8GB WB IT V1.0A
    Serial#: 12594391

    BuildInfo:
      - ELE firmware version 2.0.2-c110ba4b

    Net:   WARNING: no MAC address assigned for MAC0
    imx_get_mac_from_fuse: fuse read err: 0
    eth0: enetc-0 [PRIME]
    Hit any key to stop autoboot: 0
    Aquila iMX95 #
