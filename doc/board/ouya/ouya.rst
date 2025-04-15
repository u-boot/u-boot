.. SPDX-License-Identifier: GPL-2.0+

U-Boot for the Ouya Game Console (ouya)
=======================================

``DISCLAMER!`` Moving your Ouya to use U-Boot assumes replacement of the
vendor bootloader. Vendor android firmwares will no longer be able to run on the
device. This replacement IS reversible.

Quick Start
-----------

- Build U-Boot
- Process U-Boot
- Flashing U-Boot into the eMMC
- Boot
- Self Upgrading

Build U-Boot
------------

.. code-block:: bash

    $ export CROSS_COMPILE=arm-none-eabi-
    $ make ouya_defconfig
    $ make

After the build succeeds, you will obtain the final ``u-boot-dtb-tegra.bin``
image, ready for further processing.

Process U-Boot
--------------

``DISCLAMER!`` All questions related to the re-crypt work should be asked
in re-crypt repo issues. NOT HERE!

re-crypt is a tool that processes the ``u-boot-dtb-tegra.bin`` binary into form
usable by device. This process is required only on the first installation or
to recover the device in case of a failed update.

Permanent installation can be performed either by using the nv3p protocol or by
pre-loading just built U-Boot into RAM.

Processing for the NV3P protocol
********************************

.. code-block:: bash

    $ git clone https://gitlab.com/grate-driver/re-crypt.git
    $ cd re-crypt # place your u-boot-dtb-tegra.bin here
    $ ./re-crypt.py --dev ouya

The script will produce a ``repart-block.bin`` ready to flash.

Processing for pre-loaded U-Boot
********************************

The procedure is the same, but the ``--split`` argument is used with the
``re-crypt.py``. The script will produce ``bct.img`` and ``ebt.img`` ready
to flash.

Flashing U-Boot into the eMMC
-----------------------------

Permanent installation can be performed either by using the nv3p protocol or by
pre-loading just built U-Boot into RAM. Regardless of the method bct and bootloader
will end up in boot0 and boot1 partitions of eMMC.

Flashing with the NV3P protocol
*******************************

``DISCLAMER!`` All questions related to NvFlash should be asked in the proper
place. NOT HERE! Flashing U-Boot will erase all eMMC, so make a backup before!

Nv3p is a custom Nvidia protocol used to recover bricked devices. Devices can
enter it by pre-loading vendor bootloader with the Fusée Gelée.

With nv3p, ``repart-block.bin`` is used. It contains BCT and a bootloader in
encrypted state in form, which can just be written RAW at the start of eMMC.

.. code-block:: bash

    $ ./run_bootloader.sh -s T30 -t ./bct/ouya.bct -b android_bootloader.bin
    $ ./utiils/nvflash_v1.13.87205 --resume --rawdevicewrite 0 1024 repart-block.bin

When flashing is done, reboot the device.

Flashing with a pre-loaded U-Boot
*********************************

U-Boot pre-loaded into RAM acts the same as when it was booted "cold". Currently
U-Boot supports bootmenu entry fastboot, which allows to write a processed copy
of U-Boot permanently into eMMC.

While pre-loading U-Boot, interrupt bootflow by pressing ``CTRL + C`` (USB keyboard
must be plugged in before U-Boot is preloaded, else it will not work), input
``bootmenu`` from the keyboard and hit enter. The bootmenu will appear. There, select
``fastboot`` using the up and down arrows and enter key. After, on host PC, do:

.. code-block:: bash

    $ fastboot flash 0.1 bct.img
    $ fastboot flash 0.2 ebt.img
    $ fastboot reboot

Device will reboot.

Boot
----

To boot Linux, U-Boot will look for an ``extlinux.conf`` on eMMC. Additionally,
bootmenu provides entries to mount eMMC as mass storage, fastboot, reboot,
reboot RCM, poweroff, enter U-Boot console and update bootloader (check
the next chapter).

Flashing ``repart-block.bin`` eliminates vendor restrictions on eMMC and allows
the user to use/partition it in any way the user desires.

Self Upgrading
--------------

Place your ``u-boot-dtb-tegra.bin`` on the first partition of the USB. Enter
bootmenu, choose update bootloader option with Enter and U-Boot should update
itself. Once the process is completed, U-Boot will ask to press any button to reboot.
