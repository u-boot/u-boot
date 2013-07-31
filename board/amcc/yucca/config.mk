#
# (C) Copyright 2006
# Wolfgang Denk, DENX Software Engineering, wd@denx.de.
#
# SPDX-License-Identifier:	GPL-2.0+
#

#
# AMCC 440SPe Reference Platform (yucca) board
#

ifeq ($(ramsym),1)
CONFIG_SYS_TEXT_BASE = 0x07FD0000
else
CONFIG_SYS_TEXT_BASE = 0xfffb0000
endif

PLATFORM_CPPFLAGS += -DCONFIG_440=1

ifeq ($(debug),1)
PLATFORM_CPPFLAGS += -DDEBUG
endif

ifeq ($(dbcr),1)
PLATFORM_CPPFLAGS += -DCONFIG_SYS_INIT_DBCR=0x8cff0000
endif
