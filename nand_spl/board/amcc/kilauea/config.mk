#
# (C) Copyright 2007
# Stefan Roese, DENX Software Engineering, sr@denx.de.
#
# SPDX-License-Identifier:	GPL-2.0+
#
#
# AMCC 405EX Reference Platform (Kilauea) board
#

#
# CONFIG_SYS_TEXT_BASE for SPL:
#
# On 4xx platforms the SPL is located at 0xfffff000...0xffffffff,
# in the last 4kBytes of memory space in cache.
# We will copy this SPL into SDRAM since we can't access the NAND
# controller at CS0 while running from this location. So we set
# CONFIG_SYS_TEXT_BASE to starting address in SDRAM here.
#
CONFIG_SYS_TEXT_BASE = 0x00800000

# PAD_TO used to generate a 16kByte binary needed for the combined image
# -> PAD_TO = CONFIG_SYS_TEXT_BASE + 0x4000
PAD_TO	= 0x00804000

ifeq ($(debug),1)
PLATFORM_CPPFLAGS += -DDEBUG
endif

ifeq ($(dbcr),1)
PLATFORM_CPPFLAGS += -DCONFIG_SYS_INIT_DBCR=0x8cff0000
endif
