#
# (C) Copyright 2004 Freescale Semiconductor.
# Jeff Brown
#
# SPDX-License-Identifier:	GPL-2.0+
#

PLATFORM_RELFLAGS += -meabi

PLATFORM_CPPFLAGS += -ffixed-r2 -mstring
PLATFORM_CPPFLAGS += -maltivec -mabi=altivec -msoft-float
