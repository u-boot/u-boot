.. SPDX-License-Identifier: GPL-2.0+
.. sectionauthor:: Casey Connolly <casey.connolly@linaro.org>

Qualcomm debugging
==================

About this
----------

This page describes how to enable early UART and other debugging techniques
for Qualcomm boards.

Enable debug UART
-----------------

Newer boards (SDM845 and newer, those with GENI SE UART)
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

Open ``configs/qcom_defconfig`` and add the following snippet to the bottom:

	CONFIG_BAUDRATE=115200

	# Uncomment to enable UART pre-relocation
	CONFIG_DEBUG_UART=y
	CONFIG_DEBUG_UART_ANNOUNCE=y
	# This is the address of the debug-uart peripheral
	# The value here is for SDM845, other platforms will vary
	CONFIG_DEBUG_UART_BASE=0xa84000
	# Boards older than ~2018 pre-date the GENI driver and unfortunately
	# aren't supported here
	CONFIG_DEBUG_UART_MSM_GENI=y
	# For sdm845 this is the UART clock rate
	CONFIG_DEBUG_UART_CLOCK=7372800
	# Most newer boards have an oversampling value of 16 instead
	# of 32, they need the clock rate to be doubled
	#CONFIG_DEBUG_UART_CLOCK=14745600

Then build as normal (don't forget to ``make qcom_defconfig``` again).

Older boards (db410c and db820c)
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

Open ``configs/dragonboard<BOARD>_defconfig``

	CONFIG_BAUDRATE=115200
	CONFIG_DEBUG_UART=y
	CONFIG_DEBUG_UART_ANNOUNCE=y
	# db410c - 0x78b0000
	# db820c - 0x75b0000
	CONFIG_DEBUG_UART_BASE=0x75b0000
	CONFIG_DEBUG_UART_MSM=y
	CONFIG_DEBUG_UART_CLOCK=7372800
	#CONFIG_DEBUG_UART_SKIP_INIT=y

	CONFIG_LOG=y
	CONFIG_HEXDUMP=y
	CONFIG_CMD_LOG=y
	CONFIG_LOG_MAX_LEVEL=9
	CONFIG_LOG_DEFAULT_LEVEL=9
	CONFIG_LOGLEVEL=9

