.. SPDX-License-Identifier: GPL-2.0+

Experimental U-Boot SPL Support
===============================

There's some experimental support for some Amlogic SoCs, in U-Boot SPL.  It
replaces the proprietary bl2.bin blob used for DRAM init.  Currently Meson GX
SoCs (GXBB, GXL) are supported.

A subset of Amlogic boards have SPL enabled.  These boards have been tested and
are known to work to an extent.


Building Arm Trusted Firmware (TF-A)
------------------------------------

This U-Boot SPL port requires the BL31 stage of mainline Arm Trusted
Firmware-A firmware. It provides an open source implementation of secure
software for Armv8-A. Build it with:

.. code-block:: bash

    $ git clone https://git.trustedfirmware.org/TF-A/trusted-firmware-a.git
    $ cd trusted-firmware-a
    $ make CROSS_COMPILE=aarch64-linux-gnu- PLAT=your_soc AML_STDPARAMS=1

Replace ``your_soc`` with the SoC target you wish to build for. For GXBB it's
``gxbb`` and for GXL it's ``gxl``.


Building a bl30_new.bin binary
------------------------------

``bl30_new.bin`` has both ``bl30.bin`` and ``bl301.bin`` binary blobs
bundled. The former is the proper system control processor firmware and the
latter is a "plug-in" for board-specific DVFS/suspend-resume parameters. For
more info you may wish to read this page: `Pre-Generated FIP File Repo`_.

To build using the FIP file repo, simply issue the following commands:

.. code-block:: bash

    $ cd amlogic-boot-fip/your_board
    $ make bl30_new.bin


.. _`Pre-Generated FIP File Repo`: pre-generated-fip.rst


U-Boot compilation
------------------

U-Boot SPL is not enabled by default, instead there are config fragments that
can be used to enable it, with per-board configuration:

- ``spl-libretech-cc-1gb.config``: 1 GB LePotato board
- ``spl-libretech-cc-2gb.config``: 2 GB LePotato board
- ``spl-odroid-c2.config``: ODROID-C2
- ``spl-videostrong-kii-pro.config``: Videostrong KII Pro

Pick one of them then:

.. code-block:: bash

    $ export CROSS_COMPILE=aarch64-linux-gnu-
    $ export BL31=path/to/tf-a/bl31.bin                 # Upstream TF-A BL31 binary
    $ export SCP=path/to/bl30_new.bin                   # bl30_new.bin binary
    $ make <yourboardname>_defconfig spl-<yourboardname>.config
    $ make

Write to SD:

.. code-block:: bash

    $ DEV=/dev/boot_device
    $ dd if=u-boot-meson-with-spl.bin of=$DEV conv=fsync,notrunc bs=512 seek=1

