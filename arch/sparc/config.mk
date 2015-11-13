#
# (C) Copyright 2015
# Daniel Hellstrom, Cobham Gaisler, daniel@gaisler.com.
#
# SPDX-License-Identifier:	GPL-2.0+
#

ifeq ($(CROSS_COMPILE),)
CROSS_COMPILE := sparc-linux-
endif

# This GCC compiler is known to work:
#  https://www.kernel.org/pub/tools/crosstool/files/bin/x86_64/4.9.0/

gcclibdir := $(shell dirname `$(CC) -print-libgcc-file-name`)

CONFIG_STANDALONE_LOAD_ADDR ?= 0x00000000 -L $(gcclibdir) \
			       -T $(srctree)/examples/standalone/sparc.lds

cpuflags-$(CONFIG_LEON2) := -mcpu=leon
cpuflags-$(CONFIG_LEON3) := -mcpu=leon3

PLATFORM_CPPFLAGS += $(cpuflags-y)

PLATFORM_RELFLAGS += -fPIC
