.. SPDX-License-Identifier: GPL-2.0+

Retronix R-Car Gen4 V4H Sparrow Hawk board
==========================================

- Retronix R-Car V4H Sparrow Hawk board: https://www.retronix.com.tw/en/product_sbc.html
- Retronix R-Car V4H Sparrow Hawk documenation: https://rcar-community.github.io/Sparrow-Hawk/index.html

Build U-Boot
------------

Please follow :doc:`Renesas 64-bit ARM SoC build environment setup <build-env-aarch64>`
to correctly set up the build environment before attempting to build U-Boot.

Clone up to date U-Boot source code and change directory into the
newly cloned source directory:

.. code-block:: console

    $ git clone https://git.u-boot-project.org/u-boot/u-boot.git
    $ cd u-boot

Configure U-Boot:

.. code-block:: console

    $ make r8a779g3_sparrowhawk_defconfig

Compile U-Boot:

.. code-block:: console

    $ make

To speed up build process, -jN option can be passed to make to start
multiple jobs at the same time, this is beneficial especially on SMP
systems. The following example starts up to number of CPUs in the
system jobs, which is the recommended amount:

.. code-block:: console

    $ make -j$(nproc)

Install U-Boot
--------------

In order to install U-Boot using write into SPI NOR, first build U-Boot
for this target and collect ``flash.bin`` build artifact. Then start the
target, drop into U-Boot shell, and load the build artifact into DRAM at
well known address:

.. code-block:: console

    => tftp 0x50000000 flash.bin

Finally, write U-Boot into SPI NOR:

.. code-block:: console

    => sf probe && sf update 0x50000000 0 ${filesize}

SPI NOR layout
--------------

The Retronix R-Car V4H Sparrow Hawk board 64 MiB SPI NOR layout is listed below.

.. table:: Retronix R-Car Gen4 V4H Sparrow Hawk SPI NOR layout

   +----------------+----------------+-----------+---------------------------------------------------------------+
   | Start          | End            | Size      | Content                                                       |
   +================+================+===========+===============================================================+
   | ``0x000_0000`` | ``0x02f_ffff`` |  3072 kiB | U-Boot bootloader ``flash.bin``                               |
   +----------------+----------------+-----------+---------------------------------------------------------------+
   | ``0x030_0000`` | ``0x033_ffff`` |   256 kiB | PCIe controller firmware ``rcar_gen4_pcie.bin`` (32 kiB used) |
   +----------------+----------------+-----------+---------------------------------------------------------------+
   | ``0x034_0000`` | ``0x3f7_ffff`` | 61696 kiB | UNUSED                                                        |
   +----------------+----------------+-----------+---------------------------------------------------------------+
   | ``0x3f8_0000`` | ``0x3fb_ffff`` |   256 kiB | U-Boot bootloader environment copy 1                          |
   +----------------+----------------+-----------+---------------------------------------------------------------+
   | ``0x3fc_0000`` | ``0x3ff_ffff`` |   256 kiB | U-Boot bootloader environment copy 2                          |
   +----------------+----------------+-----------+---------------------------------------------------------------+

Bundle TFA BL31 into Linux kernel fitImage
------------------------------------------

The Retronix R-Car V4H Sparrow Hawk board starts both TFA BL31 and Linux
kernel from U-Boot. Both TFA BL31, Linux kernel and DT blob have to be
bundled into the fitImage.

Perform the following steps to build TFA at least v2.14.y:

.. code-block:: console

    $ git clone https://github.com/ARM-software/arm-trusted-firmware.git
    $ cd arm-trusted-firmware
    $ make -j$(nproc) bl31 \
        PLAT=rcar_gen4 ARCH=aarch64 LSI=V4H SPD=none \
        CTX_INCLUDE_AARCH32_REGS=0 MBEDTLS_COMMON_MK=1 \
        PTP_NONSECURE_ACCESS=1 LOG_LEVEL=20 DEBUG=0 \
        ENABLE_ASSERTIONS=0

The bundling is done using U-Boot ``mkimage`` tool:

.. code-block:: console

    mkimage \
        -f auto -E -A arm64 -C none -e 0x50200000 -a 0x50200000 \
        -d /path/to/linux/arch/arm64/boot/Image \
        -b /path/to/linux/arch/arm64/boot/dts/renesas/r8a779g3-sparrow-hawk.dtb \
        -y /path/to/arm-trusted-firmware/build/rcar_gen4/*/bl31.bin \
        -Y 0x46400000 \
        /path/to/output/fitImage
