#
# (C) Copyright 2000-2010
# Wolfgang Denk, DENX Software Engineering, wd@denx.de.
#
# SPDX-License-Identifier:	GPL-2.0+
#

PLATFORM_RELFLAGS += -meabi

PLATFORM_CPPFLAGS += -DCONFIG_8260 -DCONFIG_CPM2 -ffixed-r2 \
		     -mstring -mcpu=603e -mmultiple
