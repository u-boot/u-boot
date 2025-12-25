.. SPDX-License-Identifier: GPL-2.0+

Pine64 Star64
=============

U-Boot for the Star64 uses the same U-Boot binaries as the VisionFive 2 board.
In U-Boot SPL the actual board is detected and the device-tree patched
accordingly.

Device-tree selection
---------------------

U-Boot will set variable $fdtfile to starfive/jh7110-pine64-star64.dtb.

To overrule this selection the variable can be set manually and saved in the
environment

::

    env set fdtfile my_device-tree.dtb
    env save

or the configuration variable CONFIG_DEFAULT_FDT_FILE can be used to set to
provide a default value.

Serial Number and MAC address issues
------------------------------------

U-Boot requires valid EEPROM data to determine which board-specific fix-up to
apply at runtime. This affects the size of memory initialized, network mac
address numbering, and tuning of the network PHYs.

The Star64 does not currently ship with unique serial numbers per-device.
Devices follow a pattern where the last mac address bytes are a sum of 0x7558
and the serial number (lower port mac0), or a sum of 0x7559 and the serial
number (upper port mac1).

As tested there are several 4gb model units where the serial number and network
mac addresses collide with other devices (serial
``STAR64V1-2310-D004E000-00000005``, MACs ``6c:cf:39:00:75:61``,
``6c:cf:39:00:75:62``)

Some early Star64 boards shipped with an uninitialized EEPROM and no write
protect pull-up resistor in place. Later units of all 4gb and 8gb models
sharing the same serial number in EEPROM data will have this problem that the
network mac addresses are alike between different models and this may be
corrected by defeating the write protect resistor to write new values. As an
alternative to this, it may be worked around by overriding the mac addresses
via U-Boot environment variables.

It is required for any unit having uninitialized EEPROM and recommended for
all later Star64 4gb model units (not properly serialized) to have decided on a
new 6-byte serial number. This serial number should be high enough to
avoid collision with other JH7110 boards and low enough not to overflow i.e.
between ``cafe00`` and ``f00d00``.

Update EEPROM values
^^^^^^^^^^^^^^^^^^^^

1. Prepare EEPROM data in memory

::

	## When there is no error to load existing data:
	mac read_eeprom

	## When there is an error to load non-existing data:
	# "DRAM:  Not a StarFive EEPROM data format - magic error"
	mac initialize

2. Set Star64 values

::

	## Common values
	mac vendor PINE64
	mac pcb_revision c1
	mac bom_revision A

	## Device-specific values
	# Year 2023 week 10 production date, 8GB DRAM, optional eMMC, serial cdef01
	mac product_id STAR64V1-2310-D008E000-00cdef01

	# Last three bytes mac0: 0x7558 + serial number 0xcdef01
	mac mac0_address 6c:cf:39:ce:64:59

	# Last three bytes mac1: 0x7559 + serial number 0xcdef01
	mac mac1_address 6c:cf:39:ce:64:5a

3. Defeat write-protect pull-up resistor (if installed) and write to EEPROM

::

	mac write_eeprom

Set Variables in U-Boot
^^^^^^^^^^^^^^^^^^^^^^^

.. note:: Changing just the serial number will not alter your MAC address

The MAC addresses may be "set" as follows by writing as a custom config to SPI
(Change the last 3 bytes of MAC addreses as appropriate):

::

	env set serial# STAR64V1-2310-D008E000-00cdef01
	env set ethaddr 6c:cf:39:ce:64:59
	env set eth1addr 6c:cf:39:ce:64:5a
	env save
	reset

.. include:: jh7110_common.rst
