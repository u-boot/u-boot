#
# (C) Copyright 2003
# Wolfgang Denk, DENX Software Engineering, <wd@denx.de>
#
# SPDX-License-Identifier:	GPL-2.0+
#

#
# Default optimization level for MIPS64
#
# Note: Toolchains with binutils prior to v2.16
# are no longer supported by U-Boot MIPS tree!
#
MIPSFLAGS = -march=mips64

PLATFORM_CPPFLAGS += $(MIPSFLAGS)
PLATFORM_CPPFLAGS += -mabi=64 -DCONFIG_64BIT
ifdef CONFIG_SYS_BIG_ENDIAN
PLATFORM_LDFLAGS  += -m elf64btsmip
else
PLATFORM_LDFLAGS  += -m elf64ltsmip
endif

CONFIG_STANDALONE_LOAD_ADDR ?= 0xffffffff80200000 -T mips64.lds
