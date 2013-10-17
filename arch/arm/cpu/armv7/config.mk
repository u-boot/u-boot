#
# (C) Copyright 2002
# Gary Jennejohn, DENX Software Engineering, <garyj@denx.de>
#
# SPDX-License-Identifier:	GPL-2.0+
#

# If armv7-a is not supported by GCC fall-back to armv5, which is
# supported by more tool-chains
PF_CPPFLAGS_ARMV7 := $(call cc-option, -march=armv7-a, -march=armv5)
PLATFORM_CPPFLAGS += $(PF_CPPFLAGS_ARMV7)

# =========================================================================
#
# Supply options according to compiler version
#
# =========================================================================
PF_RELFLAGS_SLB_AT := $(call cc-option,-mshort-load-bytes,$(call cc-option,-malignment-traps,))
PLATFORM_RELFLAGS += $(PF_RELFLAGS_SLB_AT)

# SEE README.arm-unaligned-accesses
PF_NO_UNALIGNED := $(call cc-option, -mno-unaligned-access,)
PLATFORM_NO_UNALIGNED := $(PF_NO_UNALIGNED)

ifneq ($(CONFIG_IMX_CONFIG),)
ifdef CONFIG_SPL
ifdef CONFIG_SPL_BUILD
ALL-y	+= $(OBJTREE)/SPL
endif
else
ALL-y	+= $(obj)u-boot.imx
endif
endif
