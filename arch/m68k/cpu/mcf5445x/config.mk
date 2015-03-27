#
# (C) Copyright 2003 Josef Baumgartner <josef.baumgartner@telex.de>
#
# (C) Copyright 2000-2004
# Wolfgang Denk, DENX Software Engineering, wd@denx.de.
#
# Copyright 2011-2012 Freescale Semiconductor, Inc.
#
# SPDX-License-Identifier:	GPL-2.0+
#

cpuflags-$(CONFIG_MCF5441x) := -mcpu=54418 -fPIC
cpuflags-$(CONFIG_MCF5445x) := -mcpu=54455 -fPIC

ifneq (,$(findstring -linux-,$(shell $(CC) --version)))
ifneq (,$(findstring GOT,$(shell $(LD) --help)))
PLATFORM_LDFLAGS += --got=single
endif
endif
