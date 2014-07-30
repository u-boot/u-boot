#
# (C) Copyright 2000-2010
# Wolfgang Denk, DENX Software Engineering, wd@denx.de.
#
# SPDX-License-Identifier:	GPL-2.0+
#

PLATFORM_CPPFLAGS += -mstring -msoft-float

cfg=$(srctree)/include/configs/$(CONFIG_SYS_CONFIG_NAME:"%"=%).h
is440:=$(shell grep CONFIG_440 $(cfg))

ifneq (,$(findstring CONFIG_440,$(is440)))
PLATFORM_CPPFLAGS += -Wa,-m440 -mcpu=440
else
PLATFORM_CPPFLAGS += -Wa,-m405 -mcpu=405
endif
