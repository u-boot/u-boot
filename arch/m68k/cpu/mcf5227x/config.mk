#
# (C) Copyright 2003 Josef Baumgartner <josef.baumgartner@telex.de>
#
# (C) Copyright 2000-2004
# Wolfgang Denk, DENX Software Engineering, wd@denx.de.
#
# SPDX-License-Identifier:	GPL-2.0+
#

cpuflags-$(CONFIG_M52277) := -mcpu=52277 -fPIC

PLATFORM_CPPFLAGS += $(cpuflags-y)
