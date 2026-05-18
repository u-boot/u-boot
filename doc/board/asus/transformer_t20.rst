.. SPDX-License-Identifier: GPL-2.0+

U-Boot for the ASUS Eee Pad Transformer device family
=====================================================

``DISCLAMER!`` Moving your ASUS Eee Pad Transformer/Slider to use U-Boot
assumes replacement of the vendor ASUS bootloader. Vendor Android firmwares
will no longer be able to run on the device. This replacement IS reversible
if you have backups.

Quick Start
-----------

- Build U-Boot
- Process U-Boot
- Flashing U-Boot into the eMMC
- Boot
- Self Upgrading

Build U-Boot
------------

Device support is implemented by applying config fragment to a generic board
defconfig. Valid fragments are ``tf101.config``, ``tf101g.config`` and
``sl101.config``.

.. code-block:: bash

    $ export CROSS_COMPILE=arm-none-eabi-
    $ make transformer_t20_defconfig tf101.config # For TF101
    $ make

After the build succeeds, you will obtain the final ``u-boot-dtb-tegra.bin``
image, ready for further processing.

Process U-Boot
--------------

``DISCLAMER!`` All questions related to the re-crypt work should be asked
in re-crypt repo issues. NOT HERE!

re-crypt is a tool that processes the ``u-boot-dtb-tegra.bin`` binary into form
usable by device. This process is required only on the first installation or
to recover the device in case of a failed update. You need to know your device
individual SBK to continue.

.. code-block:: bash

    $ git clone https://gitlab.com/grate-driver/re-crypt.git
    $ cd re-crypt # place your u-boot-dtb-tegra.bin here
    $ ./re-crypt.py --dev tf101v1 --split # or tf101v2 or sl101 as --dev

The script will produce ``bct.img`` and ``ebt.img`` ready to flash.

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

    $ ./utils/nvflash_t20 --setbct --bct ./bct/<dev>.bct --configfile ./utils/flash.cfg
        --bl u-boot-dtb-tegra.bin --sbk <your sbk> --sync

Where <dev> is your devie codename, either ``tf101`` or ``sl101`` and <your sbk> is SBK
of your device in the form ``0xXXXXXXXX`` ``0xXXXXXXXX`` ``0xXXXXXXXX`` ``0xXXXXXXXX``

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

Flashing ``bct.img`` and ``ebt.img`` eliminates vendor restrictions on eMMC and
allows the user to use/partition it in any way the user desires.

Self Upgrading
--------------

Place your ``u-boot-dtb-tegra.bin`` on the first partition of the MicroSD card
and insert it into the tablet. Enter bootmenu, choose update the bootloader
option with the Power button and U-Boot should update itself. Once the process
is completed, U-Boot will ask to press any button to reboot.
