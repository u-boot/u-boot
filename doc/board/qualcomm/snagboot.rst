.. SPDX-License-Identifier: GPL-2.0+

Snagboot Mode for Qualcomm Platforms
====================================

`Snagboot`_ is a recovery and reflashing tool that can download a
bootloader to a device in its boot ROM recovery mode and then flash
images to it. On Qualcomm SoCs, snagrecover uses the Sahara protocol in
EDL mode to load XBL, which initializes DDR, and then loads U-Boot into
DDR. U-Boot enters fastboot mode and snagflash uses fastboot to flash
the system images. See the `Snagboot documentation`_ for how to use
the host tools.

U-Boot built in snagboot mode runs directly from the address XBL loads
it to. It must cope with the MMU left enabled by XBL, and it cannot
rely on PSCI firmware or Command DB being set up.

Currently supported platforms:

* Lemans-EVK, also known as IQ9075 (``qcom_lemans_snagboot_defconfig``)

The IQ9075 board setup and firmware are described in the "Qualcomm IQ9075"
sections of the `snagrecover documentation`_.

.. _Snagboot: https://github.com/bootlin/snagboot
.. _Snagboot documentation: https://github.com/bootlin/snagboot/tree/main/docs
.. _snagrecover documentation: https://github.com/bootlin/snagboot/blob/main/docs/snagrecover.md

Building
--------

First, setup ``CROSS_COMPILE`` for aarch64. Then, configure and build U-Boot for
the Lemans EVK platform::

    $ export CROSS_COMPILE=<aarch64 toolchain prefix>
    $ make O=.output qcom_lemans_snagboot_defconfig
    $ make -j8 O=.output

Additional tooling
^^^^^^^^^^^^^^^^^^

In order to generate a working image, ``swiv_build_utility.py`` and
``qtestsign`` are needed::

    $ git clone https://github.com/qualcomm-linux/boot-firmware-ci
    $ export SWIV_BUILD="$PWD/boot-firmware-ci/tools/swiv_build_utility.py"
    $ git clone https://github.com/msm8916-mainline/qtestsign
    $ export QTESTSIGN="$PWD/qtestsign/qtestsign.py"

``qtestsign`` requires Python 3.7+ and the ``cryptography`` module, which
can be installed with ``pip install ./qtestsign``.

Signing
^^^^^^^

Add the SWIV segment required for TZ-based signing::

    $ python3 "$SWIV_BUILD" .output/u-boot-swiv.elf .output/u-boot.elf lemans

Then sign the resulting ELF with test keys::

    $ python3 "$QTESTSIGN" -v 6 -o .output/u-boot.mbn tz .output/u-boot-swiv.elf

This generates the final ``.output/u-boot.mbn`` binary, that can then be
used with snagboot.

Board Enablement
----------------

Common snagboot settings live in the ``configs/qcom-snagboot.config``
fragment. A platform-specific snagboot defconfig includes it on top of
``qcom_defconfig`` and sets the platform values::

    #include "qcom_defconfig"
    #include "qcom-snagboot.config"

    # Address where XBL loads U-Boot
    CONFIG_TEXT_BASE=0x1c100000
    CONFIG_REMAKE_ELF=y
    CONFIG_FASTBOOT_BUF_ADDR=0xdb300000
    CONFIG_DEFAULT_DEVICE_TREE="qcom/<your-board>"

    # Timer frequency for your platform
    CONFIG_COUNTER_FREQUENCY=19200000
