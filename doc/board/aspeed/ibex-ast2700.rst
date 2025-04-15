.. SPDX-License-Identifier: GPL-2.0+

IBex AST2700
============

AST2700 integrates an IBex RISC-V 32-bits CPU as the boot MCU to execute the
first stage bootlaoder, namely SPL.

Build
-----

1. Prepare the toolchains and make sure the $PATH to toolchains is correct.
2. Use `make ibex-ast2700_defconfig` in u-boot root to build the image

Running U-Boot SPL
------------------

The U-Boot SPL will boot in M mode and load the FIT image which includes
the 2nd stage bootloaders executed by the main processor Cortex-A35.


Burn U-Boot to SPI Flash
------------------------

Use SPI flash programmer (e.g. SF100) to program the u-book-spl.bin with the
offset 0x80 bytes to the SPI flash beginning.
