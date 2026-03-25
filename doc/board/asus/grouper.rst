.. SPDX-License-Identifier: GPL-2.0+

U-Boot for the ASUS/Google Nexus 7 (2012)
=========================================

``DISCLAMER!`` Moving your ASUS/Google Nexus 7 (2012) to use U-Boot assumes
replacement of the vendor ASUS bootloader. Vendor android firmwares will no
longer be able to run on the device. This replacement IS reversible if you
have backups.

Quick Start
-----------

- Build U-Boot
- Process U-Boot
- Flashing U-Boot into the eMMC
- Boot
- Self Upgrading

Build U-Boot
------------

U-Boot features ability to detect grouper board revision on which it is
loaded. Currently are supported both TI and MAXIM based WiFi-only models
along with cellular one.

.. code-block:: bash

    $ export CROSS_COMPILE=arm-none-eabi-
    $ make grouper_defconfig # For all grouper versions and tilapia
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
    $ ./re-crypt.py --dev grouper --sbk <your sbk> --split # or --dev tilapia

where SBK has next form ``0xXXXXXXXX`` ``0xXXXXXXXX`` ``0xXXXXXXXX`` ``0xXXXXXXXX``

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

    $ ./run_bootloader.sh -s T30 -t ./bct/grouper.bct --b u-boot-dtb-tegra.bin # or tilapia.bct

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

To boot Linux, U-Boot will look for an ``extlinux.conf`` on eMMC. Additionally,
if the Volume Down button is pressed while booting, the device will enter
bootmenu. Bootmenu contains entries to mount eMMC as mass storage, fastboot,
reboot, reboot RCM, poweroff, enter U-Boot console and update bootloader (check
the next chapter).

Flashing ``bct.img`` and ``ebt.img`` eliminates vendor restrictions on eMMC and
allows the user to use/partition it in any way the user desires.

Self Upgrading
--------------

Place your ``u-boot-dtb-tegra.bin`` on the first partition of the eMMC (using
ability of u-boot to mount it). Enter bootmenu, choose update bootloader option
with Power button and U-Boot should update itself. Once the process is
completed, U-Boot will ask to press any button to reboot.
