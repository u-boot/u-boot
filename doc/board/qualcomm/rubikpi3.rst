.. SPDX-License-Identifier: GPL-2.0+
.. sectionauthor:: Hongyang Zhao <hongyang.zhao@thundersoft.com>

Thundercomm RUBIK Pi 3
======================

The RUBIK Pi 3 is a development board based on the Qualcomm QCS6490 SoC,
with 8 GiB LPDDR4x memory and 128 GiB UFS 2.2 storage. More information
can be found on `Thundercomm's product page`_.

U-Boot can be used as a replacement for Qualcomm's original EDK2 bootloader
by flashing it directly to the ``uefi_a`` (or ``uefi_b``) partition.

.. _Thundercomm's product page: https://www.thundercomm.com/rubik-pi-3/en/docs/about-rubikpi/

Installation
------------

First, set up ``CROSS_COMPILE`` for aarch64. Then, build U-Boot for
``qcom_qcs6490_rubikpi3``::

  $ export CROSS_COMPILE=<aarch64 toolchain prefix>
  $ make qcom_qcs6490_rubikpi3_defconfig
  $ make -j8

This will build ``u-boot.mbn`` in the configured output directory.
Which can be installed to the ``uefi_a`` partition on your device
with ``fastboot flash uefi_a u-boot.mbn``.

U-Boot should be running after a reboot (``fastboot reboot``).

Note that fastboot may not be available after replacing the original firmware.
To restore the original firmware, or to install a new version of U-Boot, EDL
mode can be used with the firehose loader binary appropriate for the board.

To enter EDL mode manually, disconnect the power supply and USB-C cable, press
and hold the EDL button, connect the power supply, then connect the USB-C cable
while still holding the EDL button. Wait until the host detects the board as a
Qualcomm 9008 device, then release the button. See `Thundercomm's EDL
instructions`_ for the connector and button locations.

If the stock Ubuntu image is still running, EDL mode can also be entered from a
UART, SSH, or local terminal::

  $ sudo reboot edl

A tool like bkerler's `edl`_ can be used for flashing::

  $ edl.py --loader /path/to/prog_firehose_ddr.elf w uefi_a u-boot.mbn

.. _edl: https://github.com/bkerler/edl
.. _Thundercomm's EDL instructions: https://www.thundercomm.com/rubik-pi-3/en/docs/rubik-pi-3-user-manual/1.0.0-u/set-up-your-device#enter-into-edl-mode
