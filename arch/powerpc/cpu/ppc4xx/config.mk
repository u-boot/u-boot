#
# (C) Copyright 2000-2010
# Wolfgang Denk, DENX Software Engineering, wd@denx.de.
#
# SPDX-License-Identifier:	GPL-2.0+
#

PLATFORM_RELFLAGS += -meabi
PLATFORM_CPPFLAGS += -DCONFIG_4xx -ffixed-r2 -mstring -msoft-float

cfg=$(shell grep configs $(OBJTREE)/include/config.h | sed 's/.*<\(configs.*\)>/\1/')
is440:=$(shell grep CONFIG_440 $(TOPDIR)/include/$(cfg))

ifneq (,$(findstring CONFIG_440,$(is440)))
PLATFORM_CPPFLAGS += -Wa,-m440 -mcpu=440
else
PLATFORM_CPPFLAGS += -Wa,-m405 -mcpu=405
endif
