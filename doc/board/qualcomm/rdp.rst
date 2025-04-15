.. SPDX-License-Identifier: GPL-2.0
.. sectionauthor:: Varadarajan Narayanan <quic_varada@quicinc.com>

Qualcomm Reference Design Platform (RDP)
========================================

Qualcomm RDPs are development boards based on the Qualcomm IPQ series of
SoCs. These SoCs are used as the application processors in WiFi router
platforms. RDPs come in multiple variants with differences in storage
medium (NOR, NAND, MMC), no. of USB and PCIe ports, n/w ports etc.

.. _Qualcomm's product page: https://www.qualcomm.com/products/internet-of-things/networking/wi-fi-networks/networking-pro-series/qualcomm-networking-pro-820-platform

Installation
------------
First, setup ``CROSS_COMPILE`` for aarch64. Then, build U-Boot for ``IPQ9574``::

  $ export CROSS_COMPILE=<aarch64 toolchain prefix>
  $ make qcom_ipq9574_mmc_defconfig
  $ make -j8

This will build ``u-boot.elf`` in the configured output directory.

Although the RDPs do not have secure boot set up by default, the firmware still
expects firmware ELF images to be "signed". The signature does not provide any
security in this case, but it provides the firmware with some required metadata.

To "sign" ``u-boot.elf`` you can use e.g. `qtestsign`_::

  $ qtestsign -v6 aboot -o u-boot.mbn u-boot.elf

Then install the resulting ``u-boot.mbn`` to the ``0:APPSBL`` partition
on your device with::

  IPQ9574# tftpboot path/to/u-boot.mbn
  IPQ9574# mmc part (note down the start & end block no.s of '0:APPSBL' partition)
  IPQ9574# mmc erase <start blk no> <count>
  IPQ9574# mmc write $fileaddr <blk no> <count>

U-Boot should be running after a reboot (``reset``).

.. WARNING
	Boards with newer software versions would automatically go the emergency
	download (EDL) mode if U-Boot is not functioning as expected. If its a
	runtime failure at Uboot, the system will get reset (due to watchdog)
	and XBL will try to boot from next bank and if Bank B also doesn't have
	a functional image and is not booting fine, then the system will enter
	EDL.  A tool like bkerler's `edl`_ can be used for flashing with the
	firehose loader binary appropriate for the board.

	Note that the support added is very basic. Restoring the original U-Boot
	on boards with older version of the software requires a debugger.

.. _qtestsign: https://github.com/msm8916-mainline/qtestsign
.. _edl: https://github.com/bkerler/edl
