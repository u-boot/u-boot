#
# (C) Copyright 2003
# Martin Winistoerfer, martinwinistoerfer@gmx.ch.
#
# SPDX-License-Identifier:	GPL-2.0+
#

PLATFORM_RELFLAGS +=	-meabi

PLATFORM_CPPFLAGS +=	-DCONFIG_5xx -ffixed-r2 -mpowerpc -msoft-float
