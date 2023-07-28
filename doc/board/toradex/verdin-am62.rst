.. SPDX-License-Identifier: GPL-2.0-or-later
.. sectionauthor:: Marcel Ziswiler <marcel.ziswiler@toradex.com>

Verdin AM62 Module
==================

Quick Start
-----------

- Get the binary-only SYSFW
- Get binary-only TI Linux firmware
- Build the ARM trusted firmware binary
- Build the OPTEE binary
- Build U-Boot for the R5
- Build U-Boot for the A53
- Flash to eMMC
- Boot

For an overview of the TI AM62 SoC boot flow please head over to:
.. file:: ../ti/am62x_sk.rst

Get the SYSFW
-------------

.. code-block:: bash

    $ echo "Downloading SYSFW..."
    $ git clone git://git.ti.com/k3-image-gen/k3-image-gen.git

Get the TI Linux Firmware
-------------------------

.. code-block:: bash

    $ echo "Downloading TI Linux Firmware..."
    $ git clone -b ti-linux-firmware git://git.ti.com/processor-firmware/ti-linux-firmware.git

Get and Build the ARM Trusted Firmware (Trusted Firmware A)
-----------------------------------------------------------

.. code-block:: bash

    $ echo "Downloading and building TF-A..."
    $ git clone https://github.com/ARM-software/arm-trusted-firmware.git
    $ cd arm-trusted-firmware

Then build ATF (TF-A):

.. code-block:: bash

    $ export ARCH=aarch64
    $ export CROSS_COMPILE=aarch64-none-linux-gnu-
    $ make PLAT=k3 TARGET_BOARD=lite SPD=opteed

Get and Build OPTEE
-------------------

.. code-block:: bash

    $ echo "Downloading and building OPTEE..."
    $ git clone https://github.com/OP-TEE/optee_os.git
    $ cd optee_os

Then build OPTEE:

.. code-block:: bash

    $ export CROSS_COMPILE=arm-none-linux-gnueabihf-
    $ export CROSS_COMPILE64=aarch64-none-linux-gnu-
    $ make PLATFORM=k3 CFG_ARM64_core=y

Build U-Boot for R5
-------------------

.. code-block:: bash

    $ export ARCH=arm
    $ export CROSS_COMPILE=arm-none-linux-gnueabihf-
    $ make verdin-am62_r5_defconfig
    $ make BINMAN_INDIRS=<path/to/ti-linux-firmware>

Build U-Boot for A53
--------------------

.. code-block:: bash

    $ export ARCH=arm64
    $ export CROSS_COMPILE=aarch64-none-linux-gnu-
    $ make verdin-am62_a53_defconfig
    $ make BL31=<path to ATF dir>/build/k3/lite/release/bl31.bin \
        TEE=<path to OPTEE OS dir>/out/arm-plat-k3/core/tee-pager_v2.bin \
        BINMAN_INDIRS=<path/to/ti-linux-firmware>

Flash to eMMC
-------------

.. code-block:: bash

    => mmc dev 0 1
    => fatload mmc 1 ${loadaddr} tiboot3.bin
    => mmc write ${loadaddr} 0x0 0x400
    => fatload mmc 1 ${loadaddr} tispl.bin
    => mmc write ${loadaddr} 0x400 0x1000
    => fatload mmc 1 ${loadaddr} u-boot.img
    => mmc write ${loadaddr} 0x1400 0x2000

Boot
----

Output:

.. code-block:: bash

U-Boot SPL 2023.07-00559-g523f64a2a40 (Jul 26 2023 - 17:55:45 +0200)
SYSFW ABI: 3.1 (firmware rev 0x0009 '9.0.1--v09.00.01 (Kool Koala)')
SPL initial stack usage: 13368 bytes
Trying to boot from MMC1
Authentication passed
Authentication passed
Authentication passed
Authentication passed
Authentication passed
Starting ATF on ARM64 core...

NOTICE:  BL31: v2.9(release):v2.9.0-73-g463655cc8
NOTICE:  BL31: Built : 14:51:42, Jun  5 2023
I/TC:
I/TC: OP-TEE version: 3.21.0-168-g322cf9e33 (gcc version 12.2.1 20221205 (Arm GNU Toolchain 12.2.Rel1 (Build arm-12.24))) #2 Mon Jun  5 13:04:15 UTC 2023 aarch64
I/TC: WARNING: This OP-TEE configuration might be insecure!
I/TC: WARNING: Please check https://optee.readthedocs.io/en/latest/architecture/porting_guidelines.html
I/TC: Primary CPU initializing
I/TC: SYSFW ABI: 3.1 (firmware rev 0x0009 '9.0.1--v09.00.01 (Kool Koala)')
I/TC: HUK Initialized
I/TC: Primary CPU switching to normal world boot

U-Boot SPL 2023.07-00559-g523f64a2a40 (Jul 26 2023 - 17:56:56 +0200)
SYSFW ABI: 3.1 (firmware rev 0x0009 '9.0.1--v09.00.01 (Kool Koala)')
SPL initial stack usage: 1840 bytes
Trying to boot from MMC1
Authentication passed
Authentication passed


U-Boot 2023.07-00559-g523f64a2a40 (Jul 26 2023 - 17:56:56 +0200)

SoC:   AM62X SR1.0 HS-FS
DRAM:  2 GiB
Core:  140 devices, 29 uclasses, devicetree: separate
MMC:   mmc@fa10000: 0, mmc@fa00000: 1
Loading Environment from MMC... OK
In:    serial@2800000
Out:   serial@2800000
Err:   serial@2800000
Model: Toradex 0076 Verdin AM62 Quad 2GB WB IT V1.0A
Serial#: 15037380
Carrier: Toradex Verdin Development Board V1.1A, Serial# 10754333
am65_cpsw_nuss ethernet@8000000: K3 CPSW: nuss_ver: 0x6BA01103 cpsw_ver: 0x6BA81103 ale_ver: 0x00290105 Ports:2 mdio_freq:1000000
Setting variant to wifi
Net:
Warning: ethernet@8000000port@1 MAC addresses don't match:
Address in ROM is		1c:63:49:07:f5:13
Address in environment is	00:14:2d:e5:73:c4
eth0: ethernet@8000000port@1 [PRIME]
Warning: ethernet@8000000port@2 MAC addresses don't match:
Address in ROM is		1c:63:49:07:f5:13
Address in environment is	00:14:2d:f5:73:c4
, eth1: ethernet@8000000port@2
Hit any key to stop autoboot:  0
Verdin AM62 #
