#
# (C) Copyright 2003 Josef Baumgartner <josef.baumgartner@telex.de>
#
# (C) Copyright 2000-2004
# Wolfgang Denk, DENX Software Engineering, wd@denx.de.
#
# SPDX-License-Identifier:	GPL-2.0+
#

cpuflags-$(CONFIG_MCF547x_8x) := -mcpu=5485 -fPIC

PLATFORM_CPPFLAGS += $(cpuflags-y)

ifneq (,$(findstring -linux-,$(shell $(CC) --version)))
ifneq (,$(findstring GOT,$(shell $(LD) --help)))
PLATFORM_LDFLAGS += --got=single
endif
endif
