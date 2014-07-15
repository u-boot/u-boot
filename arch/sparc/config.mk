#
# (C) Copyright 2007
# Daniel Hellstrom, Gaisler Research, daniel@gaisler.com
#
# SPDX-License-Identifier:	GPL-2.0+
#

ifeq ($(CROSS_COMPILE),)
CROSS_COMPILE := sparc-elf-
endif

gcclibdir := $(shell dirname `$(CC) -print-libgcc-file-name`)

CONFIG_STANDALONE_LOAD_ADDR ?= 0x00000000 -L $(gcclibdir) \
			       -T $(srctree)/examples/standalone/sparc.lds

PLATFORM_CPPFLAGS += -DCONFIG_SPARC -D__sparc__
