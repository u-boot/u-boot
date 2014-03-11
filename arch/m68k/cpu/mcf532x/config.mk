#
# (C) Copyright 2003 Josef Baumgartner <josef.baumgartner@telex.de>
#
# (C) Copyright 2000-2004
# Wolfgang Denk, DENX Software Engineering, wd@denx.de.
#
# SPDX-License-Identifier:	GPL-2.0+
#

cfg=$(shell grep configs $(objtree)/include/config.h | sed 's/.*<\(configs.*\)>/\1/')
is5301x:=$(shell grep CONFIG_MCF5301x $(srctree)/include/$(cfg))
is532x:=$(shell grep CONFIG_MCF532x $(srctree)/include/$(cfg))

ifneq (,$(findstring CONFIG_MCF5301x,$(is5301x)))
PLATFORM_CPPFLAGS += -mcpu=53015 -fPIC
endif
ifneq (,$(findstring CONFIG_MCF532x,$(is532x)))
PLATFORM_CPPFLAGS += -mcpu=5329 -fPIC
endif
