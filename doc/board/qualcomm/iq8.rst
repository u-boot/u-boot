.. SPDX-License-Identifier: GPL-2.0
.. sectionauthor:: Balaji Selvanathan <balaji.selvanathan@oss.qualcomm.com>

Qualcomm DragonWing IQ8
========================================

The Dragonwing IQ8 Series (which includes QCS8300) powers computeheavy and AI-based devices, and is designed
to operate in an expanded temperature range with available built-in safety features.
Dragonwing IQ8 Series delivers industrial-grade AI performance of up to 40 TOPS, an octa-core
Qualcomm Kryo Gen 6 CPU, a powerful Qualcomm Adreno 623 GPU, support for
up to 12 concurrent cameras, and 4K video encode and decode alongside multiple displays.

More information can be found on the `Qualcomm's IQ8 product page`_.

.. _Qualcomm's IQ8 product page: https://docs.qualcomm.com/bundle/publicresource/87-83839-1_REV_A_Qualcomm_IQ8_Series_Product_Brief________.pdf

Installation
------------
First, setup ``CROSS_COMPILE`` for aarch64. Then, build U-Boot for ``QCS8300``::

  $ export CROSS_COMPILE=<aarch64 toolchain prefix>
  $ make qcom_qcs8300_defconfig
  $ make -j8 u-boot.mbn

This will build the signed ``u-boot.mbn`` in the configured output directory.
The firmware expects firmware ELF images to be "signed". The signature
does not provide any security in this case, but it provides the firmware
with some required metadata.

Then flash the resulting ``u-boot.mbn`` to the ``uefi_a`` partition
on your device with ``fastboot flash uefi_a u-boot.mbn``.

U-Boot should be running after a reboot (``fastboot reboot``).

Note that fastboot is not yet supported in U-Boot on Dragonwing IQ8, as a result, to flash
back the original firmware, or new versoins of the U-Boot, EDL mode must be used.

A tool like bkerler's `edl`_ can be used for flashing.

$ edl.py --loader /path/to/prog_firehose_ddr.elf w uefi_a u-boot.mbn

.. _qtestsign: https://github.com/msm8916-mainline/qtestsign
.. _edl: https://github.com/bkerler/edl
