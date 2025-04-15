.. SPDX-License-Identifier: GPL-2.0+

U-Boot for the ASUS Transformer device family
=============================================

Quick Start
-----------

- Build U-Boot
- Boot U-Boot by loading it into RAM (coldboot)
- Chainloading U-Boot from the vendor bootloader
- Boot

Build U-Boot
------------

U-Boot can be built in two forms: U-Boot with SPL, which is used for booting
by loading directly into RAM and U-Boot without SPL, which can be flashed
and chainloaded from the vendor bootloader.

To build U-Boot with SPL proseed:

.. code-block:: bash

    $ export CROSS_COMPILE=arm-none-eabi-
    $ make tf701t_defconfig
    $ make

After the build succeeds, you will obtain the final ``u-boot-dtb-tegra.bin``
file, ready for cold booting by loading into RAM.

To build U-Boot without SPL adjust tf701t_defconfig:

.. code-block::

  CONFIG_TEXT_BASE=0x80A00000
  CONFIG_SKIP_LOWLEVEL_INIT=y
  # CONFIG_OF_BOARD_SETUP is not set
  CONFIG_TEGRA_PRAM=y

After the build succeeds, you will obtain the final ``u-boot-dtb.bin`` file,
ready for booting with fastboot boot or which can be further processed into
a flashable boot.img.

Boot U-Boot by loading it into RAM (coldboot)
---------------------------------------------

Done fairly simply by using fusee-tools (using run_bootloader.sh) and placing
``u-boot-dtb-tegra.bin`` generated on the previous step into fusee-tools dir.
This method requires constant access to the host PC or payloader and can fully
eliminate influence of the vendor bootloader onto the boot process.

.. code-block:: bash

    $ ./run_bootloader.sh -s T114 -t ./bct/tf701t.bct

Chainloading U-Boot from the vendor bootloader
----------------------------------------------

``u-boot-dtb.bin`` has to be further packed into Android boot image form,
where ``u-boot-dtb.bin`` acts as kernel, while dtb and ramdisk parts should
not be included. Then the generated boot image can be flashed into the /boot
partition of the tablet using vendor bootloader's fastboot and will act as
the bootloader of the last stage.

Boot
----
In both cases after U-Boot obtains control it performs search of extlinux.conf
first on the dock USB device is available, then on MicroSD card if available
and lastly on eMMC. If none of the devices above are present, then the device
is turned off.

If during boot of U-Boot Volume Down button is pressed, the device will enter
U-Boot bootmenu.
