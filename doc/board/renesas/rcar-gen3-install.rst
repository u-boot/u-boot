.. SPDX-License-Identifier: GPL-2.0+

Renesas R-Car Gen3 U-Boot installation
======================================

U-Boot can be installed on R-Car Gen3 systems in multiple ways.

The generic installation method, which also requires physical hardware
access, is to install U-Boot using the `flash_writer`_ tool. This
procedure is documented below.

Installation into the HyperFlash can be performed from U-Boot, please
refer to :doc:`Renesas R-Car Gen3 U-Boot HyperFlash installation <rcar-gen3-install-hf>`
for details.

Installation into the SPI NOR can be performed from U-Boot, please
refer to :doc:`Renesas R-Car Gen3 U-Boot SPI NOR installation <rcar-gen3-install-sf>`
for details.

.. note::

    The maximum u-boot.bin size for this target is 1 MiB or 0x100000 Bytes

Install U-Boot using flash_writer tool
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

In order to install U-Boot using flash_writer tool, first build U-Boot
for this target and collect ``u-boot-elf.srec`` build artifact. Then
follow the `application note`_ shipped together with the flash_writer
tool to install the ``u-boot-elf.srec`` artifact using `XLS2 command`_.

.. _`flash_writer`: https://github.com/renesas-rcar/flash_writer
.. _`application note`: https://github.com/renesas-rcar/flash_writer/blob/rcar_gen3/docs/application-note.md
.. _`XLS2 command`: https://github.com/renesas-rcar/flash_writer/blob/rcar_gen3/docs/application-note.md#341-write-to-the-s-record-format-images-to-the-serial-flash
