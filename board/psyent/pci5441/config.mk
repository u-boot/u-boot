#
# (C) Copyright 2004, Psyent Corporation <www.psyent.com>
# Scott McNutt <smcnutt@psyent.com>
#
# SPDX-License-Identifier:	GPL-2.0+
#

CONFIG_SYS_TEXT_BASE = 0x018e0000

PLATFORM_CPPFLAGS += -mno-hw-div -mno-hw-mul
PLATFORM_CPPFLAGS += -I$(TOPDIR)/board/$(VENDOR)/include

ifeq ($(debug),1)
PLATFORM_CPPFLAGS += -DDEBUG
endif
