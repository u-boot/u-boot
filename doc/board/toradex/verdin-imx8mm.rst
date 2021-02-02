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
    $ git clone https://git.trustedfirmware.org/TF-A/trusted-firmware-a.git
    $ cd trusted-firmware-a

Then build ATF (TF-A):

.. code-block:: bash

    $ make PLAT=imx8mm IMX_BOOT_UART_BASE=0x30860000 bl31
    $ cp build/imx8mm/release/bl31.bin ../

Get the DDR Firmware
--------------------

.. code-block:: bash

    $ cd ..
    $ wget https://www.nxp.com/lgfiles/NMG/MAD/YOCTO/firmware-imx-8.4.1.bin
    $ chmod +x firmware-imx-8.4.1.bin
    $ ./firmware-imx-8.4.1.bin
    $ cp firmware-imx-8.4.1/firmware/ddr/synopsys/lpddr4*.bin ./

Build U-Boot
------------
.. code-block:: bash

    $ export CROSS_COMPILE=aarch64-linux-gnu-
    $ export ATF_LOAD_ADDR=0x920000
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
