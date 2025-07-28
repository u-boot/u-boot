.. SPDX-License-Identifier: GPL-2.0+
.. sectionauthor:: Casey Connolly <casey.connolly@linaro.org>

Qualcomm Robotics RB3 Gen 2
===========================

The RB3 Gen 2 is a development board based on the Qualcomm QCM6490 SoC (a derivative
of SC7280). More information can be found on `Qualcomm's product page`_.

U-Boot can be used as a replacement for Qualcomm's original EDK2 bootloader by
flashing it directly to the uefi_a (or _b) partition.

.. _Qualcomm's product page: https://www.qualcomm.com/developer/hardware/rb3-gen-2-development-kit

Installation
------------
First, setup ``CROSS_COMPILE`` for aarch64. Then, build U-Boot for ``qcm6490``::

  $ export CROSS_COMPILE=<aarch64 toolchain prefix>
  $ make qcm6490_defconfig
  $ make -j8

This will build ``u-boot.elf`` in the configured output directory.

Although the RB3 Gen 2 does not have secure boot set up by default,
the firmware still expects firmware ELF images to be "signed". The signature
does not provide any security in this case, but it provides the firmware with
some required metadata.

To "sign" ``u-boot.elf`` you can use e.g. `qtestsign`_::

  $ qtestsign -v6 aboot -o u-boot.mbn u-boot.elf

Then install the resulting ``u-boot.mbn`` to the ``uefi_a`` partition
on your device with ``fastboot flash uefi_a u-boot.mbn``.

U-Boot should be running after a reboot (``fastboot reboot``).

Note that fastboot is not yet supported in U-Boot on this board, as a result,
to flash back the original firmware, or new versoins of the U-Boot, EDL mode
must be used. This can be accessed by pressing the EDL mode button as described
in the Qualcomm Linux documentation. A tool like bkerler's `edl`_ can be used
for flashing with the firehose loader binary appropriate for the board.

.. _qtestsign: https://github.com/msm8916-mainline/qtestsign
.. _edl: https://github.com/bkerler/edl

Usage
-----

The USB Type-A ports are connected via a PCIe USB hub, which is not supported yet.
However, the Type-C port can be used with a powered USB dock to connect peripherals
like a USB stick.
