.. SPDX-License-Identifier: GPL-2.0-or-later or CC-BY-4.0

.. Copyright (c) 2014-2026 TQ-Systems GmbH <u-boot@ew.tq-group.com>,
.. D-82229 Seefeld, Germany.

########################################
U-Boot for the TQ-Systems TQMa6x modules
########################################

The following hardware revisions are supported:

Modules:

+--------+---------------------+
| TQMa6x | REV.020x - Rev.040x |
+--------+---------------------+

Mainboards:

+--------+----------+
| MBa6x  | REV.020x |
+--------+----------+

Hardware on modules TQMa6x

- eMMC
- RTC
- PMIC
- SPI-NOR (optional)
- EEPROM
- Temperature sensor
- RAM (512 MB / 1 GB /2 GB)

Supported hardware on Starterkit MBa6x:

- 2 Ethernet PHY connected to FEC0 / FEC1
- SD-card slot
- UART
- USB

Note: To change the Ethernet port to use for networking functionality, use the
U-Boot generic environment variable ``ethact``.

.. code-block:: bash

    setenv ethact <FEC>

***************
Important notes
***************

- USB Ethernet (LAN9500 / X12) is only usable after ``usb start`` and with valid MAC in ``eth1addr`` environment variable.

***********
Boot source
***********

- SD/eMMC
- USB/SDP (with NXP UUU tool)
- SPI NOR (functional but requires additional prepended NXP header.
  Not supported in U-Boot.)

********
Building
********

To build U-Boot for the TQ-Systems TQMa6 modules:

.. code-block:: bash

	make <som>_<baseboard>_[<boot>_]defconfig
	make


**som**  is a placeholder for the module base variant:

+---------+-------------------+------------+
| tqma6qp | TQMa6QP / TQMa6DP | i.MX6QP/DP |
+---------+-------------------+------------+
| tqma6q  | TQMa6Q / TQMa6D   | i.MX6Q/D   |
+---------+-------------------+------------+
| tqma6dl | TQMa6U            | i.MX6DL    |
+---------+-------------------+------------+
| tqma6s  | TQMa6S            | i.MX6S     |
+---------+-------------------+------------+

**baseboard** is a placeholder for the mainboard to compile for:

+------+---------+
| mba6 | MBa6x   |
+------+---------+

**boot** is a optional placeholder for the boot device:

+------+-------------+
|      | SD/eMMC/SPI |
+------+-------------+
| uuu  | USB/SDP     |
+------+-------------+

************
Support Wiki
************

See `TQ Embedded Wiki for TQMa6x <https://support.tq-group.com/en/arm/tqma6x>`_.
