.. SPDX-License-Identifier: GPL-2.0+

Verdin iMX8M Mini Module
========================

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
    $ git clone -b imx_4.14.98_2.3.0 \
      https://source.codeaurora.org/external/imx/imx-atf
    $ cd imx-atf

Please edit ``plat/imx/imx8mm/include/platform_def.h`` so it contains proper
values for UART configuration and BL31 base address (correct values listed
below):

.. code-block:: bash

    #define BL31_BASE                   0x910000
    #define IMX_BOOT_UART_BASE          0x30860000
    #define DEBUG_CONSOLE               1

Then build ATF (TF-A):

.. code-block:: bash

    $ make PLAT=imx8mm bl31

Get the DDR Firmware
--------------------

.. code-block:: bash

    $ wget https://www.nxp.com/lgfiles/NMG/MAD/YOCTO/firmware-imx-8.4.1.bin
    $ chmod +x firmware-imx-8.4.1.bin
    $ ./firmware-imx-8.4.1.bin
    $ cp firmware-imx-8.4.1/firmware/ddr/synopsys/lpddr4*.bin ./

Build U-Boot
------------
.. code-block:: bash

    $ export CROSS_COMPILE=aarch64-linux-gnu-
    $ make verdin-imx8mm_defconfig
    $ make flash.bin

Flash to eMMC
-------------

.. code-block:: bash

    > tftpboot ${loadaddr} flash.bin
    > setexpr blkcnt ${filesize} + 0x1ff && setexpr blkcnt ${blkcnt} / 0x200
    > mmc dev 0 1 && mmc write ${loadaddr} 0x2 ${blkcnt}

As a convenience, instead of the last two commands one may also use the update
U-Boot wrapper:

.. code-block:: bash

    > run update_uboot

Boot
----

ATF, U-Boot proper and u-boot.dtb images are packed into FIT image,
which is loaded and parsed by SPL.

Boot sequence is:

* SPL ---> ATF (TF-A) ---> U-Boot proper

Output:

.. code-block:: bash

    U-Boot SPL 2020.01-00187-gd411d164e5 (Jan 26 2020 - 04:47:26 +0100)
    Normal Boot
    Trying to boot from MMC1
    NOTICE:  Configuring TZASC380
    NOTICE:  RDC off
    NOTICE:  BL31: v2.0(release):rel_imx_4.14.98_2.3.0-0-g09c5cc994-dirty
    NOTICE:  BL31: Built : 01:11:41, Jan 25 2020
    NOTICE:  sip svc init


    U-Boot 2020.01-00187-gd411d164e5 (Jan 26 2020 - 04:47:26 +0100)

    CPU:   Freescale i.MX8MMQ rev1.0 at 0 MHz
    Reset cause: POR
    DRAM:  2 GiB
    MMC:   FSL_SDHC: 0, FSL_SDHC: 1, FSL_SDHC: 2
    Loading Environment from MMC... OK
    In:    serial
    Out:   serial
    Err:   serial
    Model: Toradex Verdin iMX8M Mini Quad 2GB Wi-Fi / BT IT V1.0A, Serial:
    Net:   eth0: ethernet@30be0000
    Hit any key to stop autoboot:  0
    Verdin iMX8MM #
