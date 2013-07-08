#
# (C) Copyright 2005, Psyent Corporation <www.psyent.com>
# Scott McNutt <smcnutt@psyent.com>
#
# SPDX-License-Identifier:	GPL-2.0+
#

# we get text_base from board config header, so do not use this
#CONFIG_SYS_TEXT_BASE = do-not-use-me

PLATFORM_CPPFLAGS += -mno-hw-div -mno-hw-mul
PLATFORM_CPPFLAGS += -I$(TOPDIR)/board/$(VENDOR)/include

ifeq ($(debug),1)
PLATFORM_CPPFLAGS += -DDEBUG
endif
