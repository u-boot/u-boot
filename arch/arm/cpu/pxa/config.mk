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

#
# !WARNING!
# The PXA's OneNAND SPL uses .text.0 and .text.1 segments to allow booting from
# really small OneNAND memories where the mmap'd window is only 1KiB big. The
# .text.0 contains only the bare minimum needed to load the real SPL into SRAM.
# Add .text.0 and .text.1 into OBJFLAGS, so when the SPL is being objcopy'd,
# they are not discarded.
#

#ifdef CONFIG_SPL_BUILD
OBJCFLAGS += -j .text.0 -j .text.1
#endif
