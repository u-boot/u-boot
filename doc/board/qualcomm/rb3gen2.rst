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
  $ make -j8 DEVICE_TREE=qcom/qcs6490-rb3gen2

This will build ``u-boot.mbn`` in the configured output directory.

Although the board does not have secure boot set up by default,
the firmware still expects firmware ELF images to be "signed" in the MBN format.
This is handled automatically with mkmbn (see :doc:`signing` for more details).

Then install the resulting ``u-boot.mbn`` to the ``uefi_a`` partition
on your device with ``fastboot flash uefi_a u-boot.mbn``.

U-Boot should be running after a reboot (``fastboot reboot``).

Note that fastboot is not yet supported in U-Boot on this board, as a result, to flash
back the original firmware, or new versoins of the U-Boot, EDL mode must be used. This
can be accessed by holding the EDL button while powering on as described in the
Qualcomm Linux documentation.

A tool like bkerler's `edl`_ can be used for flashing with the firehose loader from the `RB3 Gen 2 bootbinaries`. ::

  $ edl.py --loader /path/to/prog_firehose_ddr.elf w uefi_a u-boot.mbn

.. _edl: https://github.com/bkerler/edl
.. _RB3 Gen 2 bootbinaries: https://artifacts.codelinaro.org/artifactory/qli-ci/software/chip/qualcomm_linux-spf-1-0/qualcomm-linux-spf-1-0_test_device_public/r1.0_00039.2/QCM6490.LE.1.0/common/build/ufs/bin/QCM6490_bootbinaries.zip

Usage
-----

The USB Type-A ports are connected via a PCIe USB hub, which is not supported yet.
However, the Type-C port can be used with a powered USB dock to connect peripherals
like a USB stick.
