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
- Modules still carrying the WinCE bootloader

Build U-Boot
------------

.. code-block:: console

    $ export CROSS_COMPILE=arm-linux-gnueabi-
    $ make colibri_vf_defconfig
    $ make

This produces ``u-boot.imx``, the image with the IVT for the Vybrid
BootROM, ready to be written to the ``u-boot`` NAND partition.

Flash to NAND from a running U-Boot
-----------------------------------

Load the image over TFTP or from a FAT formatted SD card and use the
``update_uboot`` script from the default environment:

.. code-block:: none

    Colibri VFxx # fatload mmc 0:1 ${loadaddr} u-boot.imx
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

Modules still carrying the WinCE bootloader
-------------------------------------------

Modules that were shipped with the WinCE-era Toradex bootloader (eBoot)
offer a ``flashloader`` command that writes a U-Boot image to NAND:

.. code-block:: none

    > flashloader u-boot.imx
    > reboot
