#
# (C) Copyright 2008
# Stefan Roese, DENX Software Engineering, sr@denx.de.
#
# SPDX-License-Identifier:	GPL-2.0+
#
#
# AMCC 460EX Reference Platform (Canyonlands) board
#

#
# CONFIG_SYS_TEXT_BASE for SPL:
#
# On 460EX platforms the SPL is located at 0xfffff000...0xffffffff,
# in the last 4kBytes of memory space in cache.
# We will copy this SPL into internal SRAM in start.S. So we set
# CONFIG_SYS_TEXT_BASE to starting address in internal SRAM here.
#
CONFIG_SYS_TEXT_BASE = 0xE3003000

# PAD_TO used to generate a 128kByte binary needed for the combined image
# -> PAD_TO = CONFIG_SYS_TEXT_BASE + 0x20000
PAD_TO	= 0xE3023000

PLATFORM_CPPFLAGS += -DCONFIG_440=1

ifeq ($(debug),1)
PLATFORM_CPPFLAGS += -DDEBUG
endif

ifeq ($(dbcr),1)
PLATFORM_CPPFLAGS += -DCONFIG_SYS_INIT_DBCR=0x8cff0000
endif
