#
# (C) Copyright 2007
# Stefan Roese, DENX Software Engineering, sr@denx.de.
#
# SPDX-License-Identifier:	GPL-2.0+
#
#
# AMCC 440EP Reference Platform (Bamboo) board
#

#
# CONFIG_SYS_TEXT_BASE for SPL:
#
# On 440EP(x) platforms the SPL is located at 0xfffff000...0xffffffff,
# in the last 4kBytes of memory space in cache.
# We will copy this SPL into instruction-cache in start.S. So we set
# CONFIG_SYS_TEXT_BASE to starting address in i-cache here.
#
CONFIG_SYS_TEXT_BASE = 0x00800000

# PAD_TO used to generate a 16kByte binary needed for the combined image
# -> PAD_TO = CONFIG_SYS_TEXT_BASE + 0x4000
PAD_TO	= 0x00804000

PLATFORM_CPPFLAGS += -DCONFIG_440=1

ifeq ($(debug),1)
PLATFORM_CPPFLAGS += -DDEBUG
endif

ifeq ($(dbcr),1)
PLATFORM_CPPFLAGS += -DCONFIG_SYS_INIT_DBCR=0x8cff0000
endif
