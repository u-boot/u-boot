#
# (C) Copyright 2007-2010
# Wolfgang Denk, DENX Software Engineering, wd@denx.de.
#
# SPDX-License-Identifier:	GPL-2.0+
#

ifeq ($(debug),1)
PLATFORM_CPPFLAGS += -DDEBUG
endif
