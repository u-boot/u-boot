.. SPDX-License-Identifier: GPL-2.0-or-later

SpacemiT K1 U-Boot Flash Guide
==============================

This guide explains how to flash U-Boot on SpacemiT K1 based boards. It covers
flashing images via USB fastboot.

Tested boards: Banana Pi BPI-F3, MusePi Pro.

.. note::

   This procedure flashes images to eMMC over USB fastboot. The fastboot
   function is not enabled in our SPL yet, so the download stage runs the
   SpacemiT released SPL; our built FSBL.bin and fit.itb are the images
   written to eMMC and used on the next normal boot.

Prerequisites
~~~~~~~~~~~~~

- A SpacemiT K1 board with USB Type-C and UART access
- USB-to-UART adapter (3.3V TTL)
- ``minicom`` or equivalent serial terminal, configured at 115200 8N1
- ``fastboot`` and ``flashserver`` tool on the host

Hardware Setup
~~~~~~~~~~~~~~

Refer to k1-spl.rst.

Flash images on eMMC
~~~~~~~~~~~~~~~~~~~~

**1. Obtain the release images**

Get the release package from Spacemit website. It contains SPL image, and so on.

https://archive.spacemit.com/image/k1/version/bianbu/v2.3.3/Bianbu-Minimal-K1-V2.3.3-20260128183217.zip

Unzip images and store them into a directory.

**2. Obtain flashserver tool**

Get ``flashserver`` from Spacemit website.

.. code-block:: bash

    $wget https://cdn-resource.spacemit.com/file/flash/flashserver
    $chmod +x flashserver
    $mv flashserver {flash image path}/

**3. Copy built SPL and U-Boot images**

Build U-Boot as mentioned in k1-spl.rst. Create a new directory to save.
The official u-boot.itb is used to download images. So the built U-Boot should
not replace the official one.

.. code-block:: bash

    $mkdir {flash image path}/build
    $cd {flash image path}
    $ln -sf {path to FSBL.bin} ./build/
    $ln -sf {path to u-boot.itb} ./build/fit.itb

``{path to FSBL.bin}`` is the signed FSBL produced by ``fsbl.sh`` in
k1-spl.rst, e.g. ``~/uboot-2022.10/spl_bin/FSBL.bin``.
``{path to u-boot.itb}`` is the U-Boot build output, e.g.
``~/u-boot/u-boot.itb``.

**4. Update configuration files**

The ``partition_2M.json`` and ``partition_universal.json`` files come from
the release package. Patch the ``fsbl`` and ``uboot`` entries to point at
the images staged under ``build/`` (pick the layout that matches your eMMC):

.. code-block:: diff

   diff -puNr bianbu-25/partition_2M.json clean/partition_2M.json
   --- bianbu-25/partition_2M.json	2026-03-02 11:55:58.631116807 +0800
   +++ clean/partition_2M.json	2026-05-20 11:25:21.683801401 +0800
   @@ -13,7 +13,7 @@
          "name": "fsbl",
          "offset": "128K",
          "size": "256K",
   -      "image": "factory/FSBL.bin"
   +      "image": "build/FSBL.bin"
        },
        {
          "name": "env",
   @@ -31,7 +31,7 @@
          "name": "uboot",
          "offset": "640K",
          "size": "-",
   -      "image": "u-boot.itb"
   +      "image": "build/fit.itb"
        }
      ]
    }
   diff -puNr bianbu-25/partition_universal.json clean/partition_universal.json
   --- bianbu-25/partition_universal.json	2026-03-02 11:55:58.642116862 +0800
   +++ clean/partition_universal.json	2026-05-20 11:26:23.932581853 +0800
   @@ -14,7 +14,7 @@
               "name": "fsbl",
               "offset": "128K",
               "size": "256K",
   -            "image": "factory/FSBL.bin"
   +            "image": "build/FSBL.bin"
           },
           {
               "name": "env",
   @@ -32,7 +32,7 @@
               "name": "uboot",
               "offset": "2M",
               "size": "2M",
   -            "image": "u-boot.itb"
   +            "image": "build/fit.itb"
           },
           {
               "name": "bootfs",

Deploying via USB Fastboot
~~~~~~~~~~~~~~~~~~~~~~~~~~~

To enter BootROM fastboot mode:

1. Power off the board by unplugging its power supply.
2. **Press and hold** the FDL button (called "Boot Key" on some boards;
   see the board layout above for the BPI-F3).
3. While holding the button, use a USB cable to connect the OTG port to
   your host. This cable is also used by fastboot to upload the firmware.
4. Release the button.

On the host, ``fastboot devices`` should list the board::

    dfu-device     DFU download

The serial console shows the BootROM's USB download handler trace,
including a line like::

    usb2d_initialize : enter

This indicates the board is ready to accept an image via USB.

.. tip::

   If you are worried about insufficient USB power, you can first plug
   in the power, then release the button, and then plug in the USB
   cable.

On the host:

.. code-block:: console

    $sudo ./flashserver

When ``flashserver`` is running, it lists the detected fastboot devices.
Enter the corresponding number to select one.
