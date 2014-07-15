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

# On supported platforms we set the bit which causes us to trap on unaligned
# memory access.  This is the opposite of what the compiler expects to be
# the default so we must pass in -mno-unaligned-access so that it is aware
# of our decision.
PF_NO_UNALIGNED := $(call cc-option, -mno-unaligned-access,)
PLATFORM_CPPFLAGS += $(PF_NO_UNALIGNED)
