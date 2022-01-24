.. SPDX-License-Identifier: GPL-2.0+

U-Boot for the Microsoft Surface RT tablet
==========================================

Quick Start
-----------

- Build U-Boot
- Boot

Build U-Boot
------------

.. code-block:: bash

    $ export CROSS_COMPILE=arm-linux-gnueabi-
    $ make surface-rt_defconfig
    $ make

After the build succeeds, you will obtain the final ``u-boot-dtb-tegra.bin``
image, ready for loading.

Boot
----

Currently, U-Boot can be preloaded into RAM via the Fusée Gelée. To enter
RCM protocol use ``power`` and ``volume up`` key combination from powered
off device. The host PC should recognize an APX device.

Built U-Boot ``u-boot-dtb-tegra.bin`` can be loaded from fusee-tools
directory with

.. code-block:: bash

    $ ./run_bootloader.sh -s T30 -t ./bct/surface-rt.bct

To boot Linux, U-Boot will look for an ``extlinux.conf`` on MicroSD and then on
eMMC. Additionally, if the Volume Down button is pressed while loading, the
device will enter bootmenu. Bootmenu contains entries to mount MicroSD and eMMC
as mass storage, fastboot, reboot, reboot RCM, poweroffand enter U-Boot console.
