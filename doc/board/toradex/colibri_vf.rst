.. SPDX-License-Identifier: GPL-2.0-or-later
.. sectionauthor:: Mehmet Fide <mehmet.fide@screeningeagle.com>

Colibri VF50/VF61 Modules
=========================

- SoM: https://www.toradex.com/computer-on-modules/colibri-arm-family/nxp-vybrid-vf5xx-vf6xx
- Carrier boards: https://www.toradex.com/products/carrier-board/colibri-evaluation-board
  and https://www.toradex.com/products/carrier-board/iris-carrier-board

Quick Start
-----------

- Build U-Boot
- Flash to NAND from a running U-Boot
- Flash to NAND over recovery mode
- The NAND boot control block
- Modules still carrying the WinCE bootloader

Build U-Boot
------------

.. code-block:: console

    $ export CROSS_COMPILE=arm-linux-gnueabi-
    $ make colibri_vf_defconfig
    $ make

This produces ``u-boot.imx``, the plain image with the IVT for the
Vybrid BootROM. The BootROM expects the IVT at offset 0x400 of a NAND
boot device (VFxxx Reference Manual rev. 8, table 7-51), so the image
written to the ``u-boot`` NAND partition needs 1 KiB of padding in
front of it:

.. code-block:: console

    $ make u-boot-nand.imx

The plain ``u-boot.imx`` written to NAND is refused by the BootROM,
which then silently falls back to the serial downloader.

Flash to NAND from a running U-Boot
-----------------------------------

Load the image over TFTP or from a FAT formatted SD card and use the
``update_uboot`` script from the default environment:

.. code-block:: none

    Colibri VFxx # fatload mmc 0:1 ${loadaddr} u-boot-nand.imx
    Colibri VFxx # run update_uboot
    Colibri VFxx # reset

Flash to NAND over recovery mode
--------------------------------

With nothing bootable in NAND, the Vybrid BootROM falls back to the
serial downloader on the USB client port. The `imx_usb_loader
<https://github.com/boundarydevices/imx_usb_loader>`_ tool can push
``u-boot.imx`` straight into OCRAM and run it:

.. code-block:: console

    $ imx_usb u-boot.imx

From the running U-Boot, flash NAND as in the previous section.

The NAND boot control block
---------------------------

The BootROM finds U-Boot through the Firmware Configuration Block in
the first NAND page: the ECC level and the page of the image. Modules
come with it, it needs writing again only after the first block was
erased or when the ECC layout of the NAND changes. With
``CONFIG_CMD_NANDBCB`` enabled, ``nandbcb`` writes the block the ROM
expects on the Vybrid, and ``nandbcb dump`` shows the one in NAND. The
same block holds the Toradex configuration block in its second page,
so read that page back after the rewrite:

.. code-block:: none

    Colibri VFxx # nand read ${loadaddr} 0x800 0x800
    Colibri VFxx # nandbcb bcbonly 0 0x20000 0x160000 0x20000 0x160000
    Colibri VFxx # nand write ${loadaddr} 0x800 0x800
    Colibri VFxx # nandbcb dump 0

The offsets name the ``u-boot`` partition of the default ``mtdparts``,
which starts at 0x20000, for both firmware copies the ROM knows, as the
modules come with one image; ``nandbcb`` takes the whole device as its
first argument because the block sits at page 0 of the device.

Modules still carrying the WinCE bootloader
-------------------------------------------

Modules that were shipped with the WinCE-era Toradex bootloader (eBoot)
offer a ``flashloader`` command that writes a U-Boot image to NAND:

.. code-block:: none

    > flashloader u-boot-nand.imx
    > reboot
