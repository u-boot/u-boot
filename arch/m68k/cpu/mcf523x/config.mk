#
# (C) Copyright 2003 Josef Baumgartner <josef.baumgartner@telex.de>
#
# (C) Copyright 2000-2004
# Wolfgang Denk, DENX Software Engineering, wd@denx.de.
#
# SPDX-License-Identifier:	GPL-2.0+
#

cpuflags-$(CONFIG_M5235) := -mcpu=5235 -fPIC

PLATFORM_CPPFLAGS += $(cpuflags-y)
