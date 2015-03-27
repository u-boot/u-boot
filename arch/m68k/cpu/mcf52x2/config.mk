#
# (C) Copyright 2003 Josef Baumgartner <josef.baumgartner@telex.de>
#
# (C) Copyright 2000-2004
# Wolfgang Denk, DENX Software Engineering, wd@denx.de.
#
# SPDX-License-Identifier:	GPL-2.0+
#

cpuflags-$(CONFIG_M5208) := -mcpu=5208
cpuflags-$(CONFIG_M5249) := -mcpu=5249
cpuflags-$(CONFIG_M5253) := -mcpu=5253
cpuflags-$(CONFIG_M5271) := -mcpu=5271
cpuflags-$(CONFIG_M5272) := -mcpu=5272
cpuflags-$(CONFIG_M5275) := -mcpu=5275
cpuflags-$(CONFIG_M5282) := -mcpu=5282

PLATFORM_CPPFLAGS += $(cpuflags-y)
