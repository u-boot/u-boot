#
# (C) Copyright 2006
# Stefan Roese, DENX Software Engineering, sr@denx.de.
#
# SPDX-License-Identifier:	GPL-2.0+
#
#
# AMCC 440EPx Reference Platform (Sequoia) board
#

#
# CONFIG_SYS_TEXT_BASE for SPL:
#
# On 440EP(x) platforms the SPL is located at 0xfffff000...0xffffffff,
# in the last 4kBytes of memory space in cache.
# We will copy this SPL into internal SRAM in start.S. So we set
# CONFIG_SYS_TEXT_BASE to starting address in internal SRAM here.
#
CONFIG_SYS_TEXT_BASE = 0xE0013000

# PAD_TO used to generate a 16kByte binary needed for the combined image
# -> PAD_TO = CONFIG_SYS_TEXT_BASE + 0x4000
PAD_TO	= 0xE0017000

PLATFORM_CPPFLAGS += -DCONFIG_440=1

ifeq ($(debug),1)
PLATFORM_CPPFLAGS += -DDEBUG
endif

ifeq ($(dbcr),1)
PLATFORM_CPPFLAGS += -DCONFIG_SYS_INIT_DBCR=0x8cff0000
endif
