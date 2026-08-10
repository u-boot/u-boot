.. SPDX-License-Identifier: GPL-2.0-or-later or CC-BY-4.0

.. Copyright (c) 2017-2025 TQ-Systems GmbH <u-boot@ew.tq-group.com>,
.. D-82229 Seefeld, Germany.

########################################
U-Boot for the TQ-Systems TQMa6x modules
########################################

The following hardware revisions are supported:

Modules:

+------------+----------+
| TQMa6ULx   | REV.030x |
+------------+----------+
| TQMa6ULLx  | REV.030x |
+------------+----------+
| TQMa6ULxL  | REV.030x |
+------------+----------+
| TQMa6ULLxL | REV.030x |
+------------+----------+

Mainboards:

+----------+----------+
| MBa6ULx  | REV.020x |
+----------+----------+

Hardware on modules TQMa6ULx, TQMa6ULxL, TQMa6ULLx and TQMa6ULLxL

- eMMC
- RTC
- PMIC
- SPI-NOR (optional)
- EEPROM
- Temperature sensor
- RAM (256 MiB / 512 MiB)

Supported hardware on Starterkit MBa6ULx:

- 2 Ethernet PHY connected to FEC0 / FEC1 (usage depends on CPU)
- SD-card slot
- UART
- USB

Note: To change the Ethernet port to use for networking functionality, use the
U-Boot generic environment variable ``ethact``.

.. code-block:: bash

    setenv ethact <FEC>

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

To build U-Boot for the TQ-Systems TQMa6L modules:

.. code-block:: bash

	make <som>_<baseboard>_<boot>_defconfig
	make


**som**  is a placeholder for the module base variant:

+------------+------------+----------------+
| tqma6ulx   | TQMa6ULx   | i.MX6UL        |
+------------+------------+----------------+
| tqma6ullx  | TQMa6ULLx  | i.MX6ULL       |
+------------+------------+----------------+
| tqma6ulxl  | TQMa6ULxL  | i.MX6UL (lga)  |
+------------+------------+----------------+
| tqma6ullxl | TQMa6ULLxL | i.MX6ULL (lga) |
+------------+------------+----------------+

**baseboard** is a placeholder for the mainboard to compile for:

+----------+----------+
| mba6ul   | MBa6ULx  |
+----------+----------+

**boot** is a placeholder for the boot device:

+------+---------+
| mmc  | SD/eMMC |
+------+---------+
| spi  | SPI-NOR |
+------+---------+

************
Support Wiki
************

See `TQ Embedded Wiki for TQMa6ulx <https://support.tq-group.com/en/arm/tqma6ulx>`_.
