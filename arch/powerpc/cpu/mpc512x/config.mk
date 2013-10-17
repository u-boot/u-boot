#
# (C) Copyright 2007-2010 DENX Software Engineering
#
# SPDX-License-Identifier:	GPL-2.0+
#

PLATFORM_RELFLAGS += -meabi

PLATFORM_CPPFLAGS += -DCONFIG_MPC512X -DCONFIG_E300 \
			-ffixed-r2 -msoft-float -mcpu=603e
