#
# (C) Copyright 2000-2010
# Wolfgang Denk, DENX Software Engineering, wd@denx.de.
#
# SPDX-License-Identifier:	GPL-2.0+
#

PLATFORM_CPPFLAGS += -mstring -msoft-float

ifneq (,$(CONFIG_440))
PLATFORM_CPPFLAGS += -Wa,-m440 -mcpu=440
else
PLATFORM_CPPFLAGS += -Wa,-m405 -mcpu=405
endif
