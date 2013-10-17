#
# (C) Copyright 2002
# Gary Jennejohn, DENX Software Engineering, <garyj@denx.de>
#
# SPDX-License-Identifier:	GPL-2.0+
#

PLATFORM_CPPFLAGS += -march=armv5te
# =========================================================================
#
# Supply options according to compiler version
#
# =========================================================================
PF_RELFLAGS_SLB_AT := $(call cc-option,-mshort-load-bytes,$(call cc-option,-malignment-traps,))
PLATFORM_RELFLAGS += $(PF_RELFLAGS_SLB_AT)

ifneq ($(CONFIG_IMX_CONFIG),)
ifdef CONFIG_SPL
ifdef CONFIG_SPL_BUILD
ALL-y	+= $(OBJTREE)/SPL
endif
else
ALL-y	+= $(obj)u-boot.imx
endif
endif
