.. SPDX-License-Identifier: GPL-2.0-or-later
.. sectionauthor:: Parth Pancholi <parth.pancholi@toradex.com>

Verdin AM62P Module
===================

- SoM: https://www.toradex.com/computer-on-modules/verdin-arm-family/ti-am62p
- Carrier board: https://www.toradex.com/products/carrier-board/verdin-development-board-kit

Quick Start
-----------

- Setup environment variables
- Get binary-only TI Linux firmware
- Build the ARM trusted firmware binary
- Build the OPTEE binary
- Build U-Boot for the R5
- Build U-Boot for the A53
- Flash to eMMC
- Boot

Setup environment
-----------------

Suggested current toolchains are ARM 11.3 (https://developer.arm.com/downloads/-/arm-gnu-toolchain-downloads):

- https://developer.arm.com/-/media/Files/downloads/gnu/11.3.rel1/binrel/arm-gnu-toolchain-11.3.rel1-x86_64-arm-none-linux-gnueabihf.tar.xz
- https://developer.arm.com/-/media/Files/downloads/gnu/11.3.rel1/binrel/arm-gnu-toolchain-11.3.rel1-x86_64-aarch64-none-linux-gnu.tar.xz

.. code-block:: bash

    $ export CROSS_COMPILE_32=<path/to/arm/toolchain/bin/>arm-none-linux-gnueabihf-
    $ export CROSS_COMPILE_64=<path/to/arm64/toolchain/bin/>aarch64-none-linux-gnu-

Get the TI Linux Firmware
-------------------------

.. code-block:: bash

    $ echo "Downloading TI Linux Firmware..."
    $ git clone -b ti-linux-firmware https://git.ti.com/git/processor-firmware/ti-linux-firmware.git

Get and Build the ARM Trusted Firmware (Trusted Firmware A)
-----------------------------------------------------------

.. code-block:: bash

    $ echo "Downloading and building TF-A..."
    $ git clone https://git.trustedfirmware.org/TF-A/trusted-firmware-a.git
    $ cd trusted-firmware-a

Then build ATF (TF-A):

.. code-block:: bash

    $ export CROSS_COMPILE="$CROSS_COMPILE_64"
    $ make PLAT=k3 K3_PM_SYSTEM_SUSPEND=1 TARGET_BOARD=lite SPD=opteed

Get and Build OPTEE
-------------------

.. code-block:: bash

    $ echo "Downloading and building OPTEE..."
    $ git clone https://github.com/OP-TEE/optee_os.git
    $ cd optee_os

Then build OPTEE:

.. code-block:: bash

    $ export CROSS_COMPILE="$CROSS_COMPILE_32"
    $ export CROSS_COMPILE64="$CROSS_COMPILE_64"
    $ make PLATFORM=k3-am62px CFG_ARM64_core=y

Build U-Boot for R5
-------------------

.. code-block:: bash

    $ export CROSS_COMPILE="$CROSS_COMPILE_32"
    $ export BINMAN_INDIRS=<path/to/ti-linux-firmware>
    $ make O=/tmp/verdin-am62p-r5 verdin-am62p_r5_defconfig
    $ make O=/tmp/verdin-am62p-r5

Build U-Boot for A53
--------------------

.. code-block:: bash

    $ export CROSS_COMPILE=$CROSS_COMPILE_64
    $ export BL31=<path/to/atf>/build/k3/lite/release/bl31.bin
    $ export TEE=<path/to/optee>/out/arm-plat-k3/core/tee-pager_v2.bin
    $ export BINMAN_INDIRS="<path/to/ti-linux-firmware> /tmp/verdin-am62p-r5"
    $ make O=/tmp/verdin-am62p-a53 verdin-am62p_a53_defconfig
    $ make O=/tmp/verdin-am62p-a53

Flash to eMMC
-------------

.. code-block:: console

    => mmc dev 0 1
    => fatload mmc 1 ${loadaddr} tiboot3.bin
    => mmc write ${loadaddr} 0x0 0x400
    => fatload mmc 1 ${loadaddr} tispl.bin
    => mmc write ${loadaddr} 0x400 0x1000
    => fatload mmc 1 ${loadaddr} u-boot.img
    => mmc write ${loadaddr} 0x1400 0x2000

As a convenience, instead of having to remember all those addresses and sizes,
one may also use the update U-Boot wrappers:

.. code-block:: console

    => tftpboot ${loadaddr} tiboot3.bin
    => run update_tiboot3

    => tftpboot ${loadaddr} tispl.bin
    => run update_tispl

    => tftpboot ${loadaddr} u-boot.img
    => run update_uboot

Boot
----

Output:

.. code-block:: console

U-Boot SPL 2025.04-00006-g51dc98d36470 (May 12 2025 - 15:46:57 +0100)
SYSFW ABI: 4.0 (firmware rev 0x000b '11.0.7--v11.00.07 (Fancy Rat)')
Changed A53 CPU frequency to 1250000000Hz (U grade) in DT
SPL initial stack usage: 17080 bytes
Trying to boot from MMC1
Authentication passed
Authentication passed
Authentication passed
Loading Environment from nowhere... OK
init_env from device 9 not supported!
Authentication passed
Authentication passed
Starting ATF on ARM64 core...

NOTICE:  BL31: v2.12.0(release):v2.12.0-1106-g4301798db096
NOTICE:  BL31: Built : 10:57:58, May  9 2025
I/TC:
I/TC: OP-TEE version: 4.6.0-18-g76d920d354df (gcc version 12.3.1 20230626 (Arm GNU Toolchain 12.3.Rel1 (Build arm-12.35))) #4 Tue May  6 19:48:13 UTC 2025 aarch64
I/TC: WARNING: This OP-TEE configuration might be insecure!
I/TC: WARNING: Please check https://optee.readthedocs.io/en/latest/architecture/porting_guidelines.html
I/TC: Primary CPU initializing
I/TC: GIC redistributor base address not provided
I/TC: Assuming default GIC group status and modifier
I/TC: SYSFW ABI: 4.0 (firmware rev 0x000b '11.0.7--v11.00.07 (Fancy Rat)')
I/TC: Activated SA2UL device
I/TC: Enabled firewalls for SA2UL TRNG device
I/TC: SA2UL TRNG initialized
I/TC: SA2UL Drivers initialized
I/TC: HUK Initialized
I/TC: Primary CPU switching to normal world boot

U-Boot SPL 2025.04-00006-g51dc98d36470 (May 12 2025 - 15:47:54 +0100)
SYSFW ABI: 4.0 (firmware rev 0x000b '11.0.7--v11.00.07 (Fancy Rat)')
SPL initial stack usage: 1760 bytes
HW CFG: 0x00
Trying to boot from MMC1
Authentication passed
Authentication passed


U-Boot 2025.04-00006-g51dc98d36470 (May 12 2025 - 15:47:54 +0100)

SoC:   AM62PX SR1.0 HS-FS
DRAM:  2 GiB
Core:  147 devices, 31 uclasses, devicetree: separate
MMC:   mmc@fa10000: 0, mmc@fa00000: 1
Loading Environment from MMC... Reading from MMC(0)... OK
In:    serial@2800000
Out:   serial@2800000
Err:   serial@2800000
Model: Toradex 0099 Verdin AM62P Quad 2GB WB IT V1.0A
Serial#: 15664919
Carrier: Toradex Dahlia V1.1D, Serial# 11287149
am65_cpsw_nuss ethernet@8000000: K3 CPSW: nuss_ver: 0x6BA01903 cpsw_ver: 0x6BA81903 ale_ver: 0x00290105 Ports:2
Setting variant to wifi
Net:
Warning: ethernet@8000000port@1 MAC addresses don't match:
Address in ROM is               58:a1:5f:b8:93:f9
Address in environment is       00:14:2d:ef:07:17
eth0: ethernet@8000000port@1 [PRIME]Could not get PHY for mdio@f00: addr 7
am65_cpsw_nuss_port ethernet@8000000port@2: phy_connect() failed

Hit any key to stop autoboot:  0
Verdin AM62P #

