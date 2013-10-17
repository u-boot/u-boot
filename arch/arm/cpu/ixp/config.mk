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

# =========================================================================
#
# Supply options according to compiler version
#
# =========================================================================
PF_RELFLAGS_SLB_AT := $(call cc-option,-mshort-load-bytes,$(call cc-option,-malignment-traps,))
PLATFORM_RELFLAGS += $(PF_RELFLAGS_SLB_AT)
