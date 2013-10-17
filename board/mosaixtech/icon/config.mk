#
# (C) Copyright 2009-2010
# Stefan Roese, DENX Software Engineering, sr@denx.de.
#
# SPDX-License-Identifier:	GPL-2.0+
#

PLATFORM_CPPFLAGS += -DCONFIG_440=1

ifeq ($(debug),1)
PLATFORM_CPPFLAGS += -DDEBUG
endif

ifeq ($(dbcr),1)
PLATFORM_CPPFLAGS += -DCONFIG_SYS_INIT_DBCR=0x8cff0000
endif
