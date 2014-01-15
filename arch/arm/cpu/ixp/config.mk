#
# (C) Copyright 2002
# Sysgo Real-Time Solutions, GmbH <www.elinos.com>
# Marius Groeger <mgroeger@sysgo.de>
#
# SPDX-License-Identifier:	GPL-2.0+
#

BIG_ENDIAN = y

PLATFORM_RELFLAGS += -mbig-endian

PLATFORM_CPPFLAGS += -mbig-endian -march=armv5te -mtune=strongarm1100

PLATFORM_LDFLAGS += -EB
USE_PRIVATE_LIBGCC = yes
