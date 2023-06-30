.. SPDX-License-Identifier: GPL-2.0+

U-Boot for the ASUS Transformer device family
=============================================

``DISCLAMER!`` Moving your ASUS Transformer to use U-Boot
assumes replacement of the vendor ASUS bootloader. Vendor
android firmwares will no longer be able to run on the device.
This replacement IS reversible.

Quick Start
-----------

- Build U-Boot
- Pack U-Boot into repart-block
- Flash repart-block into the eMMC
- Flash repart-block into TF600T SPI flash
- Boot
- Self Upgrading

Build U-Boot
------------

Device support is implemented by applying config fragment
to a generic board defconfig. Valid fragments are ``tf201.config``,
``tf300t.config``, ``tf300tg.config``, ``tf300tl.config``,
``tf700t.config``, ``tf600t.config`` and ``p1801-t.config``.

.. code-block:: bash

    $ export CROSS_COMPILE=arm-linux-gnueabi-
    $ make transformer_t30_defconfig tf201.config # For TF201
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
    $ ./re-crypt.sh -d tf201 -k deadbeefdeadc0dedeadd00dfee1dead

Script will produce you a `repart-block.bin` ready to flash.

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

Flash repart-block into TF600T SPI flash
----------------------------------------

Unlike other transformers TF600T uses separate 4 MB SPI flash which
contains all data required for boot. It is flashed from within u-boot
itself preloaded into RAM using fusee gelee. After creating your
``repart-block.bin`` you have to place it on a 1st partition of microSD
card formated in fat. Then insert this microSD card into your tablet
and boot it using fusee gelee and u-boot which was included into
repart-block.bin, while booting you must hold volume down button.
Process should take less then a minute, if everything goes correct,
on microSD will appear ``spi-flash-backup.bin`` file, which is dump of
your spi flash content and can be used to restore UEFI, do not loose it,
tablet will power itself off.

Self-updating of u-boot is performed by placing ``u-boot-dtb-tegra.bin``
on 1st partition of microSD, inserting it into tablet and booting with
pressed volume down button.

Boot
----

After flashing ``repart-block.bin`` the device should reboot and turn
itself off. This is normal behavior if no boot configuration is
found.

To boot Linux, U-Boot will look for an ``extlinux.conf`` on MicroSD
and then on eMMC. Additionally if Volume Down button is pressed
while booting device will enter bootmenu. Bootmenu contains entries
to mount MicroSD and eMMC as mass storage, fastboot, reboot, reboot
RCM, poweroff, enter U-Boot console and update bootloader (check next
chapter).

Flashing ``repart-block.bin`` eliminates vendor restriction on eMMC
and allows the user to use/partition it in any way the user desires.

Self Upgrading
--------------

Place your ``u-boot-dtb-tegra.bin`` on the first partition of the
MicroSD card and insert it into the tablet. Enter bootmenu, choose
update bootloader option with Power button and U-Boot should update
itself. Once the process is completed, U-Boot will ask to press any
button to reboot.
