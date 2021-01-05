.. SPDX-License-Identifier: GPL-2.0+

Apalis iMX8X V1.1A Module
==========================

Quick Start
-----------

- Build the ARM trusted firmware binary
- Get scfw_tcm.bin and ahab-container.img
- Build U-Boot
- Load U-Boot binary using uuu
- Flash U-Boot binary into the eMMC
- Boot

Get and Build the ARM Trusted Firmware
--------------------------------------

.. code-block:: bash

    $ git clone -b toradex_imx_5.4.24_2.1.0 http://git.toradex.com/cgit/imx-atf.git
    $ cd imx-atf/
    $ make PLAT=imx8qx bl31

Get scfw_tcm.bin and ahab-container.img
---------------------------------------

.. code-block:: bash

    $ wget https://github.com/toradex/i.MX-System-Controller-Firmware/blob/master/src/scfw_export_mx8qx_b0/build_mx8qx_b0/mx8qx-apalis-scfw-tcm.bin
    $ wget https://www.nxp.com/lgfiles/NMG/MAD/YOCTO/imx-seco-3.6.3.bin
    $ chmod +x imx-seco-3.6.3.bin
    $ ./imx-seco-3.6.3.bin

Copy the following binaries to the U-Boot folder:

.. code-block:: bash

    $ cp imx-atf/build/imx8qx/release/bl31.bin .
    $ cp imx-seco-3.6.3/firmware/seco/mx8qxb0-ahab-container.img mx8qx-ahab-container.imx8_defconfig

Build U-Boot
------------
.. code-block:: bash

    $ make apalis-imx8x_defconfig
    $ make u-boot-dtb.imx

Load the U-Boot Binary Using UUU
--------------------------------

Get the latest version of the universal update utility (uuu) aka ``mfgtools 3.0``:

https://community.nxp.com/external-link.jspa?url=https%3A%2F%2Fgithub.com%2FNXPmicro%2Fmfgtools%2Freleases

Put the module into USB recovery aka serial downloader mode, connect USB device
to your host and execute uuu:

.. code-block:: bash

    sudo ./uuu u-boot/u-boot-dtb.imx

Flash the U-Boot Binary into the eMMC
-------------------------------------

Burn the ``u-boot-dtb.imx`` binary to the primary eMMC hardware boot area
partition and boot:

.. code-block:: bash

    load mmc 1:1 $loadaddr u-boot-dtb.imx
    setexpr blkcnt ${filesize} + 0x1ff && setexpr blkcnt ${blkcnt} / 0x200
    mmc dev 0 1
    mmc write ${loadaddr} 0x0 ${blkcnt}
