.. SPDX-License-Identifier: CC-BY-4.0

.. Copyright (c) 2020-2026 TQ-Systems GmbH <u-boot@ew.tq-group.com>,
.. D-82229 Seefeld, Germany.

########################################
U-Boot for the TQ-Systems TQMa7x modules
########################################

This file contains information for the port of
U-Boot to the TQ-Systems TQMa7x modules.

***********
Boot source
***********

The following boot sources are supported:

- SD/eMMC
- USB (SDP)

QSPI boot is functional but requires an additional prepended NXP header
image. This currently unsupported in u-boot.

********
Building
********

To build U-Boot for the TQ-Systems TQMa7x modules:

.. code-block:: bash

	make tqma7_mba7_<boot>_defconfig
	make

**boot** is a placeholder for the boot device:

+------+-----------+
| mmc  | SD/eMMC   |
+------+-----------+
| uuu  | USB (SDP) |
+------+-----------+

The default build artifact is named ``u-boot-with-spl.imx``.

*****************************************
Serial Download Protocol (SDP) / USB boot
*****************************************

The complete system image can be programmed with ``uuu``
(https://github.com/nxp-imx/mfgtools) to eMMC.

Serial Download Protocol is supported on the Micro-B USB port (X5) of MBa7x.
The command ``fastboot usb 0`` is used to enable the fastboot gadget.

Build SDP enabled U-Boot image
==============================

.. code-block:: bash

	make <som>_mba7_uuu_defconfig
	make

Booting
=======

With UUU

.. code-block:: bash

	uuu -b spl <UUU U-Boot image>

************
Support Wiki
************

See `TQ Embedded Wiki for TQMa7x <https://support.tq-group.com/en/arm/tqma7x>`_.
