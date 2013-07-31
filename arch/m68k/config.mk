#
# (C) Copyright 2000-2002
# Wolfgang Denk, DENX Software Engineering, wd@denx.de.
#
# SPDX-License-Identifier:	GPL-2.0+
#

CROSS_COMPILE ?= m68k-elf-

CONFIG_STANDALONE_LOAD_ADDR ?= 0x20000

PLATFORM_CPPFLAGS += -DCONFIG_M68K -D__M68K__
PLATFORM_LDFLAGS  += -n
PLATFORM_RELFLAGS              += -ffunction-sections -fdata-sections
LDFLAGS_FINAL                  += --gc-sections
