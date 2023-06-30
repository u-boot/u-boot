.. SPDX-License-Identifier: GPL-2.0+

U-Boot for the ASUS/Google Nexus 7 (2012)
=========================================

``DISCLAMER!`` Moving your ASUS/Google Nexus 7 (2012) to use
U-Boot assumes replacement of the vendor ASUS bootloader. Vendor
android firmwares will no longer be able to run on the device.
This replacement IS reversible.

Quick Start
-----------

- Build U-Boot
- Pack U-Boot into repart-block
- Flash repart-block into the eMMC
- Boot
- Self Upgrading

Build U-Boot
------------

Device support is implemented by applying config fragment to a generic
board defconfig. Valid fragments are ``grouper_E1565.config``,
``grouper_PM269.config`` and ``tilapia.config``.

.. code-block:: bash

    $ export CROSS_COMPILE=arm-linux-gnueabi-
    $ make grouper_common_defconfig grouper_E1565.config # For maxim based grouper
    $ make

After the build succeeds, you will obtain the final ``u-boot-dtb-tegra.bin``
image, ready for flashing (but check the next section for additional
adjustments).

Pack U-Boot into repar-block
----------------------------

``DISCLAMER!`` All questions related to re-crypt work should be asked
in re-crypt repo issues. NOT HERE!

re-crypt is a small script which packs ``u-boot-dtb-tegra.bin`` in
form usable by device. This process is required only on the first
installation or to recover the device in case of a failed update.
You need to know your tablet's individual SBK to continue.

.. code-block:: bash

    $ git clone https://github.com/clamor-s/re-crypt.git
    $ cd re-crypt # place your u-boot-dtb-regra.bin here
    $ ./re-crypt.sh -d grouper -k deadbeefdeadc0dedeadd00dfee1dead

Script will produce you a ``repart-block.bin`` ready to flash.

Flash repart-block into the eMMC
--------------------------------

``DISCLAMER!`` All questions related to NvFlash should be asked
in the proper place. NOT HERE! Flashing repart-block will erase
all your eMMC, so make a backup before!

``repart-block.bin`` contains BCT and bootloader in encrypted state
in form which can just be written RAW at the start of eMMC.

.. code-block:: bash

    $ wheelie --blob blob.bin
    $ nvflash --resume --rawdevicewrite 0 1024 repart-block.bin

Boot
----

After flashing ``repart-block.bin`` the device should reboot and turn
itself off. This is normal behavior if no boot configuration is
found.

To boot Linux, U-Boot will look for an ``extlinux.conf`` configuration
on eMMC. Additionally if Volume Down button is pressed while booting
device will enter bootmenu. Bootmenu contains entries to mount eMMC as
mass storage, fastboot, reboot, reboot RCM, poweroff, enter U-Boot
console and update bootloader (check next chapter).

Flashing ``repart-block.bin`` eliminates vendor restriction on eMMC
and allows the user to use/partition it in any way the user desires.

Self Upgrading
--------------

Place your ``u-boot-dtb-tegra.bin`` on the first partition of the
eMMC (using ability of u-boot to mount it). Enter bootmenu, choose
update bootloader option with Power button and U-Boot should update
itself. Once the process is completed, U-Boot will ask to press any
button to reboot.
