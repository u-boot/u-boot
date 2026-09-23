.. SPDX-License-Identifier: GPL-2.0+

Renesas R-Car Gen3 U-Boot SPI NOR installation
==============================================

U-Boot can be installed on R-Car Gen3 systems into SPI NOR from U-Boot.

.. note::

    This update mechanism is only available in case the TFA has been built
    with `RCAR_RPC_HYPERFLASH_LOCKED=0 SPD=none`.

Install U-Boot into SPI NOR using write from U-Boot
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

In order to install U-Boot using write into SPI NOR, first build U-Boot
for this target and collect ``u-boot.bin`` build artifact. Then start the
target, drop into U-Boot shell, and load the build artifact into DRAM at
well known address:

.. code-block:: console

    => tftp 0x50000000 u-boot.bin

Finally, write U-Boot into SPI NOR:

.. code-block:: console

    => sf probe && sf update 0x50000000 0x640000 $filesize
