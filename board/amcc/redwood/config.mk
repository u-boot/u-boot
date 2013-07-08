#
# (C) Copyright 2008
# Feng Kan, Applied Micro Circuits Corp., fkan@amcc.com.
#
# SPDX-License-Identifier:	GPL-2.0+
#

#
# AMCC 460SX Reference Platform (redwood) board
#

PLATFORM_CPPFLAGS += -DCONFIG_440=1

ifeq ($(debug),1)
PLATFORM_CPPFLAGS += -DDEBUG
endif

ifeq ($(dbcr),1)
PLATFORM_CPPFLAGS += -DCONFIG_SYS_INIT_DBCR=0x8cff0000
endif
