#
# (C) Copyright 2011
# Julius Baxter <julius@opencores.org>
#
# SPDX-License-Identifier:	GPL-2.0+
#

CROSS_COMPILE ?= or32-elf-

# r10 used for global object pointer, already set in OR32 GCC but just to be
# clear
PLATFORM_CPPFLAGS += -DCONFIG_OPENRISC -D__OR1K__ -ffixed-r10

CONFIG_STANDALONE_LOAD_ADDR ?= 0x40000

LDSCRIPT ?= $(SRCTREE)/$(CPUDIR)/u-boot.lds
