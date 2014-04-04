#
# (C) Copyright 2002
# Wolfgang Denk, DENX Software Engineering, wd@denx.de.
#
# SPDX-License-Identifier:	GPL-2.0+
#
#
# Korat (PPC440EPx) board
#

PLATFORM_CPPFLAGS += -DCONFIG_440=1

ifeq ($(debug),1)
PLATFORM_CPPFLAGS += -DDEBUG
endif

ifeq ($(emul),1)
PLATFORM_CPPFLAGS += -fno-schedule-insns -fno-schedule-insns2
endif

ifeq ($(dbcr),1)
PLATFORM_CPPFLAGS += -DCONFIG_SYS_INIT_DBCR=0x8CFF0000
endif

ifndef CONFIG_KORAT_PERMANENT
LDSCRIPT := $(srctree)/board/$(BOARDDIR)/u-boot-F7FC.lds
endif
