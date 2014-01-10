#
# (C) Copyright 2002
# Gary Jennejohn, DENX Software Engineering, <garyj@denx.de>
#
# SPDX-License-Identifier:	GPL-2.0+
#
PLATFORM_RELFLAGS += -fno-common -ffixed-x18

# SEE README.arm-unaligned-accesses
PF_NO_UNALIGNED := $(call cc-option, -mstrict-align)
PLATFORM_NO_UNALIGNED := $(PF_NO_UNALIGNED)

PF_CPPFLAGS_ARMV8 := $(call cc-option, -march=armv8-a)
PLATFORM_CPPFLAGS += $(PF_CPPFLAGS_ARMV8)
PLATFORM_CPPFLAGS += $(PF_NO_UNALIGNED)
