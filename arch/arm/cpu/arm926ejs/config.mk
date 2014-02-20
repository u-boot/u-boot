#
# (C) Copyright 2002
# Gary Jennejohn, DENX Software Engineering, <garyj@denx.de>
#
# SPDX-License-Identifier:	GPL-2.0+
#

PLATFORM_CPPFLAGS += -march=armv5te

ifneq ($(CONFIG_IMX_CONFIG),)
ifdef CONFIG_SPL
ifdef CONFIG_SPL_BUILD
ALL-y	+= $(OBJTREE)/SPL
endif
else
ALL-y	+= u-boot.imx
endif
endif
