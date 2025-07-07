.. SPDX-License-Identifier: GPL-2.0
.. sectionauthor:: Balaji Selvanathan <balaji.selvanathan@oss.qualcomm.com>

Qualcomm DragonWing
========================================

Qualcomm DragonWing are industrial-grade boards that provides various series
of processors such as IQ6 (QCS615), IQ8 (QCS8300) and IQ9 (QCS9100).
These SoCs are used for factory/industry based applications.
More information can be found on the `Qualcomm's IQ6 product page`_,
`Qualcomm's IQ8 product page`_ and `Qualcomm's IQ9 product page`_.

.. _Qualcomm's IQ6 product page: https://docs.qualcomm.com/bundle/publicresource/87-83838-1_REV_A_Qualcomm_IQ6_Series_Product_Brief.pdf
.. _Qualcomm's IQ8 product page: https://docs.qualcomm.com/bundle/publicresource/87-83839-1_REV_A_Qualcomm_IQ8_Series_Product_Brief________.pdf
.. _Qualcomm's IQ9 product page: https://docs.qualcomm.com/bundle/publicresource/87-83840-1_REV_A_Qualcomm_IQ9_Series_Product_Brief.pdf

Installation
------------
First, setup ``CROSS_COMPILE`` for aarch64. Then, build U-Boot for ``QCS615``, ``QCS8300`` or ``QCS9100``::

  $ export CROSS_COMPILE=<aarch64 toolchain prefix>
  $ make qcom_qcs8300_defconfig
  $ make -j8 u-boot.mbn

Although the board does not have secure boot set up by default,
the firmware still expects firmware ELF images to be "signed". The signature
does not provide any security in this case, but it provides the firmware with
some required metadata.

To "sign" ``u-boot.elf`` you can use e.g. `qtestsign`_::

  $ qtestsign -v6 aboot -o u-boot.mbn u-boot.elf

Then flash the resulting ``u-boot.mbn`` to the ``uefi_a`` partition
on your device with ``fastboot flash uefi_a u-boot.mbn``.

U-Boot should be running after a reboot (``fastboot reboot``).

Note that fastboot is not yet supported in U-Boot on Dragonwing boards, as a result, to flash
back the original firmware, or new versoins of the U-Boot, EDL mode must be used.

A tool like bkerler's `edl`_ can be used for flashing with the firehose loader (for example, for QCS9100
the firehose loader can be obtained from `dragonwing IQ9 bootbinaries`.) ::

$ edl.py --loader /path/to/prog_firehose_ddr.elf w uefi_a u-boot.mbn

.. _qtestsign: https://github.com/msm8916-mainline/qtestsign
.. _edl: https://github.com/bkerler/edl
.. _dragonwing IQ9 bootbinaries: https://artifacts.codelinaro.org/ui/native/qli-ci/flashable-binaries/qimpsdk/qcs9075-rb8-core-kit
