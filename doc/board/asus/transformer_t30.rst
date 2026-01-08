.. SPDX-License-Identifier: GPL-2.0+

U-Boot for the ASUS Transformer device family
=============================================

``DISCLAMER!`` Moving your ASUS Transformer to use U-Boot assumes replacement
of the vendor ASUS bootloader. Vendor Android firmwares will no longer be
able to run on the device. This replacement IS reversible if you have backups.

Quick Start
-----------

- Build U-Boot
- Process U-Boot
- Flashing U-Boot into the eMMC
- Flashing U-Boot into the SPI flash
- Boot
- Self Upgrading

Build U-Boot
------------

U-Boot features ability to detect transformer device model on which it is
loaded. The list of supported devices include:

- ASUS Transformer Prime TF201
- ASUS Transformer Pad (3G/LTE) TF300T/TG/TL
- ASUS Transformer Infinity TF700T
- ASUS Portable AiO P1801-T
- ASUS VivoTab RT TF600T

.. code-block:: bash

    $ export CROSS_COMPILE=arm-none-eabi-
    $ make transformer_t30_defconfig
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
    $ ./re-crypt.py --dev tf201 --sbk <your sbk> --split

where SBK has next form ``0xXXXXXXXX`` ``0xXXXXXXXX`` ``0xXXXXXXXX`` ``0xXXXXXXXX``

The script will produce ``bct.img`` and ``ebt.img`` ready to flash.

``NOTE!`` If you have TF700T it may have different sizes of boot0/boot1 partitions,
re-crypt sets default boot partition size to 2MB and if you have different size
add ``--bootsize`` key with yout boot partition size in bytes to the command.

Flashing U-Boot into the eMMC
-----------------------------

``DISCLAMER!`` All questions related to fusee-tools should be asked in the proper
place. NOT HERE! Flashing U-Boot will erase all eMMC, so make a backup before!

Permanent installation can be performed by pre-loading just built U-Boot into RAM.
Bct and bootloader will end up in boot0 and boot1 partitions of eMMC.

You have to clone and prepare fusee-tools from here: https://gitlab.com/grate-driver/fusee-tools
according to fusee-tools README to continue.

Bootloader preloading is performed to device in APX/RCM mode connected to host
PC. This mode can be entered by holding ``power`` and ``volume up`` buttons on
turned off tablet connected to the host PC. Host PC should detect APX USB
device in ``lsusb``.

U-Boot pre-loaded into RAM acts the same as when it was booted "cold". Currently
U-Boot supports bootmenu entry fastboot, which allows to write a processed copy
of U-Boot permanently into eMMC. This is how U-Boot can be preloaded using
fusee-tools:

.. code-block:: bash

    $ ./run_bootloader.sh -s T30 -t ./bct/<dev>.bct --b u-boot-dtb-tegra.bin

Where <dev> is your devie codename (``tf201``, ``tf300t``, ``tf700t`` etc.).

While pre-loading U-Boot, hold the ``volume down`` button which will trigger
the bootmenu. There, select ``fastboot`` using the volume and power buttons.
After, on host PC, do:

.. code-block:: bash

    $ fastboot flash 0.1 bct.img
    $ fastboot flash 0.2 ebt.img
    $ fastboot reboot

Device will reboot.

Flashing U-Boot into the SPI Flash
----------------------------------

Some of Transformers use a separate 4 MB SPI flash, which contains all data
required for boot. It is flashed from within U-Boot itself, preloaded into RAM
using Fusée Gelée.

Create ``repart-block.bin`` using re-crypt without ``--split`` key:

.. code-block:: bash

    $ git clone https://gitlab.com/grate-driver/re-crypt.git
    $ cd re-crypt # place your u-boot-dtb-tegra.bin here
    $ ./re-crypt.py --dev tf600t --sbk <your sbk>

After creating your ``repart-block.bin`` you have to place it on a 1st partition
of microSD card formated in fat. Then insert this microSD card into your tablet
and boot it using Fusée Gelée and U-Boot, which was included into
``repart-block.bin``, while booting you must hold the ``volume down`` button.

The process should take less than a minute, if everything goes correctly,
on microSD will appear ``spi-flash-backup.bin`` file, which is the dump of your
SPI Flash content and can be used to restore UEFI, do not lose it, tablet will
power itself off.

Self-updating of U-Boot is performed by placing ``u-boot-dtb-tegra.bin`` on 1st
partition of microSD, inserting it into the tablet and booting with a pressed
``volume down`` button.

Boot
----

To boot Linux, U-Boot will look for an ``extlinux.conf`` on MicroSD and then on
eMMC. Additionally, if the Volume Down button is pressed while booting, the
device will enter bootmenu. Bootmenu contains entries to mount MicroSD and eMMC
as mass storage, fastboot, reboot, reboot RCM, poweroff, enter U-Boot console
and update bootloader (check the next chapter).

Flashing ``bct.img`` and ``ebt.img`` eliminates vendor restrictions on eMMC and
allows the user to use/partition it in any way the user desires.

Self Upgrading
--------------

Place your ``u-boot-dtb-tegra.bin`` on the first partition of the MicroSD card
and insert it into the tablet. Enter bootmenu, choose update the bootloader
option with the Power button and U-Boot should update itself. Once the process
is completed, U-Boot will ask to press any button to reboot.
