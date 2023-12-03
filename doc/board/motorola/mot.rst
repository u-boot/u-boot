.. SPDX-License-Identifier: GPL-2.0+

U-Boot for the Motorola Atrix 4G (MB860) and Droid X2 (MB870)
=============================================================

``DISCLAMER!`` Moving your device to use U-Boot assumes replacement of the
vendor bootloader. Vendor Android firmwares will no longer be able to run on
the device. This replacement IS reversible if you have backups.

Quick Start
-----------

- Prerequisites
- Build U-Boot
- Process U-Boot
- Flashing U-Boot into the eMMC
- Boot
- Self Upgrading

Prerequisites
-------------

In order to work with RCM/APX mode, both devices require a factory cable which
is made by routing 5V to the ID pin of a micro-USB cable (5v is applied to both
ID and dedicated 5v). This way, the host PC can detect the device in RCM mode,
and the device can operate without a battery.

Build U-Boot
------------

Device support is implemented by applying config fragment to a generic
board defconfig. Valid fragments are ``daytona.config`` and ``olympus.config``.

.. code-block:: bash

    $ export CROSS_COMPILE=arm-none-eabi-
    $ make mot_defconfig olympus.config # For Atrix 4G
    $ make

After the build succeeds, you will obtain the final ``u-boot-dtb-tegra.bin``
image, ready for further processing.

Process U-Boot
--------------

``DISCLAMER!`` All questions related to the re-crypt work should be asked
in re-crypt repo issues. NOT HERE!

re-crypt is a tool that processes the ``u-boot-dtb-tegra.bin`` binary into form
usable by device. This process is required only on the first installation or to
recover the device in case of a failed update. You need to know your device
individual SBK to continue.

.. code-block:: bash

    $ git clone https://gitlab.com/grate-driver/re-crypt.git
    $ cd re-crypt # place your u-boot-dtb-tegra.bin here
    $ ./re-crypt.py --dev olympus --sbk <your sbk> --split

where SBK has next form ``0xXXXXXXXX`` ``0xXXXXXXXX`` ``0xXXXXXXXX`` ``0xXXXXXXXX``

The script will produce ``bct.img`` and ``ebt.img`` ready to flash.

Flashing U-Boot into the eMMC
-----------------------------

``DISCLAMER!`` All questions related to fusee-tools should be asked in the proper
place. NOT HERE! Flashing U-Boot will erase all eMMC, so make a backup before!

U-Boot pre-loaded into RAM acts the same as when it was booted "cold". Currently
U-Boot supports bootmenu entry fastboot, which allows to write a processed copy
of U-Boot permanently into eMMC.

While pre-loading U-Boot, hold the ``volume down`` button which will trigger
the bootmenu. There, select ``fastboot`` using the volume and power buttons.
After, on host PC, do:

.. code-block:: bash

    $ fastboot flash 0.1 bct.img
    $ fastboot flash 0.2 ebt.img
    $ fastboot reboot

Device will reboot.

Boot
----

To boot Linux, U-Boot will look for an ``extlinux.conf`` on MicroSD and then on
eMMC. Additionally, if the Volume Down button is pressed while booting, the
device will enter bootmenu. Bootmenu contains entries to mount MicroSD and eMMC
as mass storage, fastboot, reboot, reboot RCM, poweroff, enter U-Boot console
and update bootloader (check the next chapter).

Flashing ``repart-block.bin`` eliminates vendor restrictions on eMMC and allows
the user to use/partition it in any way the user desires.

Self Upgrading
--------------

Place your ``u-boot-dtb-tegra.bin`` on the first partition of the MicroSD card
and insert it into the device. Enter bootmenu, choose update the bootloader
option with the Power button and U-Boot should update itself. Once the process
is completed, U-Boot will ask to press any button to reboot.
