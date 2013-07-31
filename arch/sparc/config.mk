#
# (C) Copyright 2007
# Daniel Hellstrom, Gaisler Research, daniel@gaisler.com
#
# SPDX-License-Identifier:	GPL-2.0+
#

CROSS_COMPILE ?= sparc-elf-

CONFIG_STANDALONE_LOAD_ADDR ?= 0x00000000 -L $(gcclibdir) -T sparc.lds

PLATFORM_CPPFLAGS += -DCONFIG_SPARC -D__sparc__
