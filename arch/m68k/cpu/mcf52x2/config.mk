#
# (C) Copyright 2003 Josef Baumgartner <josef.baumgartner@telex.de>
#
# (C) Copyright 2000-2004
# Wolfgang Denk, DENX Software Engineering, wd@denx.de.
#
# SPDX-License-Identifier:	GPL-2.0+
#

cfg=$(shell grep configs $(OBJTREE)/include/config.h | sed 's/.*<\(configs.*\)>/\1/')
is5208:=$(shell grep CONFIG_M5208 $(TOPDIR)/include/$(cfg))
is5249:=$(shell grep CONFIG_M5249 $(TOPDIR)/include/$(cfg))
is5253:=$(shell grep CONFIG_M5253 $(TOPDIR)/include/$(cfg))
is5271:=$(shell grep CONFIG_M5271 $(TOPDIR)/include/$(cfg))
is5272:=$(shell grep CONFIG_M5272 $(TOPDIR)/include/$(cfg))
is5275:=$(shell grep CONFIG_M5275 $(TOPDIR)/include/$(cfg))
is5282:=$(shell grep CONFIG_M5282 $(TOPDIR)/include/$(cfg))

ifneq (,$(findstring CONFIG_M5208,$(is5208)))
PLATFORM_CPPFLAGS += -mcpu=5208
endif
ifneq (,$(findstring CONFIG_M5249,$(is5249)))
PLATFORM_CPPFLAGS += -mcpu=5249
endif
ifneq (,$(findstring CONFIG_M5253,$(is5253)))
PLATFORM_CPPFLAGS += -mcpu=5253
endif
ifneq (,$(findstring CONFIG_M5271,$(is5271)))
PLATFORM_CPPFLAGS += -mcpu=5271
endif
ifneq (,$(findstring CONFIG_M5272,$(is5272)))
PLATFORM_CPPFLAGS += -mcpu=5272
endif
ifneq (,$(findstring CONFIG_M5275,$(is5275)))
PLATFORM_CPPFLAGS += -mcpu=5275
endif
ifneq (,$(findstring CONFIG_M5282,$(is5282)))
PLATFORM_CPPFLAGS += -mcpu=5282
endif
