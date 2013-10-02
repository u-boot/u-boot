#
# (C) Copyright 2002
# Sysgo Real-Time Solutions, GmbH <www.elinos.com>
# Marius Groeger <mgroeger@sysgo.de>
#
# SPDX-License-Identifier:	GPL-2.0+
#

PLATFORM_CPPFLAGS += -mcpu=xscale
# =========================================================================
#
# Supply options according to compiler version
#
# ========================================================================
PF_RELFLAGS_SLB_AT := $(call cc-option,-mshort-load-bytes,$(call cc-option,-malignment-traps,))
PLATFORM_RELFLAGS += $(PF_RELFLAGS_SLB_AT)
