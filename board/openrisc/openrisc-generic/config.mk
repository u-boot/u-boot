#
# (C) Copyright 2011, Julius Baxter <julius@opencores.org>
#
# SPDX-License-Identifier:	GPL-2.0+
#

PLATFORM_CPPFLAGS += -mhard-mul -mhard-div

ifeq ($(debug),1)
PLATFORM_CPPFLAGS += -DDEBUG
endif
