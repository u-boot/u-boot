#
# (C) Copyright 2011
# Julius Baxter <julius@opencores.org>
#
# SPDX-License-Identifier:	GPL-2.0+
#

ifeq ($(CROSS_COMPILE),)
CROSS_COMPILE := or1k-elf-
endif

# r10 used for global object pointer, already set in OR32 GCC but just to be
# clear
PLATFORM_CPPFLAGS += -D__OR1K__ -ffixed-r10

CONFIG_STANDALONE_LOAD_ADDR ?= 0x40000
