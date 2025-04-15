.. SPDX-License-Identifier: GPL-2.0+

U-Boot for the Xiaomi Mi Pad tablet
===================================

``DISCLAMER!`` Moving your Xiaomi Mi Pad to use U-Boot assumes replacement
of the vendor bootloader. Vendor Android firmwares will no longer be able
to run on the device. This replacement IS reversible.

Quick Start
-----------

- Build U-Boot
- Boot U-Boot
- Process and flash U-Boot
- Boot Linux
- Self Upgrading
- Chainload configuration

Build U-Boot
------------

.. code-block:: bash

    $ export CROSS_COMPILE=arm-none-eabi-
    $ make mocha_defconfig
    $ make

After the build succeeds, you will obtain the final ``u-boot-dtb-tegra.bin``
image, ready for booting or further processing.

Boot U-Boot
-----------
Existing tegrarcm loader can be used to pre-load U-Boot you have build
into RAM and basically perform a tethered cold-boot.

.. code-block:: bash

    $ tegrarcm --bct mocha.bct --bootloader u-boot-dtb-tegra.bin --loadaddr 0x80108000

U-Boot will try to load Linux kernel and if fails, it will turn the
tablet off. While pre-loading U-Boot, hold the ``volume down`` button
which will trigger the bootmenu.

Process and flash U-Boot
------------------------

``DISCLAMER!`` All questions related to the re-crypt work should be asked
in re-crypt repo issues. NOT HERE!

re-crypt is a tool that processes the ``u-boot-dtb-tegra.bin`` binary into
form usable by device. This process is required only on the first
installation or to recover the device in case of a failed update.

.. code-block:: bash

    $ git clone https://gitlab.com/grate-driver/re-crypt.git
    $ cd re-crypt # place your u-boot-dtb-tegra.bin here
    $ ./re-crypt.py --dev mocha

The script will produce ``bct.img`` and ``ebt.img`` ready to flash.

Permanent installation can be performed by pre-loading just built U-Boot
into RAM via tegrarcm. While pre-loading U-Boot, hold the ``volume down``
button which will trigger the bootmenu. There, select ``fastboot`` using
the volume and power buttons.

After, on host PC, do:

.. code-block:: bash

    $ fastboot flash 0.1 bct.img
    $ fastboot flash 0.2 ebt.img
    $ fastboot reboot

Device will reboot.

Boot Linux
----------

To boot Linux, U-Boot will look for an ``extlinux.conf`` on MicroSD and then on
eMMC. Additionally, if the ``volume down`` button is pressed while booting, the
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

Chainload configuration
-----------------------

To build U-Boot without SPL suitable for chainloading adjust mocha_defconfig:

.. code-block::

  CONFIG_TEXT_BASE=0x80A00000
  CONFIG_SKIP_LOWLEVEL_INIT=y
  # CONFIG_OF_BOARD_SETUP is not set
  CONFIG_TEGRA_PRAM=y

After the build succeeds, you will obtain the final ``u-boot-dtb.bin``
file, ready for booting using vendor bootloader's fastboot or which can be
further processed into a flashable image.
