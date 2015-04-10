#
# (C) Copyright 2002
# Gary Jennejohn, DENX Software Engineering, <garyj@denx.de>
#
# SPDX-License-Identifier:	GPL-2.0+
#
PLATFORM_RELFLAGS += -fno-common -ffixed-x18

PF_NO_UNALIGNED := $(call cc-option, -mstrict-align)
PLATFORM_CPPFLAGS += $(PF_NO_UNALIGNED)
