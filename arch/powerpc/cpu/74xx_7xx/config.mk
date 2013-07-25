#
# (C) Copyright 2001
# Josh Huber <huber@mclx.com>, Mission Critical Linux, Inc.
#
# SPDX-License-Identifier:	GPL-2.0+
#

PLATFORM_RELFLAGS += -meabi

PLATFORM_CPPFLAGS += -DCONFIG_74xx_7xx -ffixed-r2 -mstring
