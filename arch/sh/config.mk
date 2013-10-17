#
# (C) Copyright 2000-2002
# Wolfgang Denk, DENX Software Engineering, wd@denx.de.
#
# SPDX-License-Identifier:	GPL-2.0+
#

CROSS_COMPILE ?= sh4-linux-

CONFIG_STANDALONE_LOAD_ADDR ?= 0x8C000000
ifeq ($(CPU),sh2)
CONFIG_STANDALONE_LOAD_ADDR += -EB
endif

PLATFORM_CPPFLAGS += -DCONFIG_SH -D__SH__
PLATFORM_LDFLAGS += -e $(CONFIG_SYS_TEXT_BASE) --defsym reloc_dst=$(CONFIG_SYS_TEXT_BASE)
LDFLAGS_FINAL = --gc-sections
