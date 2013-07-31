#
# (C) Copyright 2005
# Sandburst Corporation
#
# SPDX-License-Identifier:	GPL-2.0+
#

#
# Sandburst Corporation Metrobox Reference Design
# Travis B. Sawyer
#

PLATFORM_CPPFLAGS += -DCONFIG_440=1

ifeq ($(debug),1)
PLATFORM_CPPFLAGS += -DDEBUG
endif

ifeq ($(dbcr),1)
PLATFORM_CPPFLAGS += -DCONFIG_SYS_INIT_DBCR=0x8cff0000
endif
