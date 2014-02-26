#
# (C) Copyright 2000-2002
# Wolfgang Denk, DENX Software Engineering, wd@denx.de.
#
# SPDX-License-Identifier:	GPL-2.0+
#

CONFIG_STANDALONE_LOAD_ADDR ?= 0x40000

PLATFORM_CPPFLAGS += -fno-strict-aliasing
PLATFORM_CPPFLAGS += -mregparm=3
PLATFORM_CPPFLAGS += -fomit-frame-pointer
PF_CPPFLAGS_X86   := $(call cc-option, -fno-toplevel-reorder, \
		       $(call cc-option, -fno-unit-at-a-time)) \
		     $(call cc-option, -mpreferred-stack-boundary=2)
PLATFORM_CPPFLAGS += $(PF_CPPFLAGS_X86)
PLATFORM_CPPFLAGS += -fno-dwarf2-cfi-asm
PLATFORM_CPPFLAGS += -DREALMODE_BASE=0x7c0

# Support generic board on x86
__HAVE_ARCH_GENERIC_BOARD := y

PLATFORM_RELFLAGS += -ffunction-sections -fvisibility=hidden

PLATFORM_LDFLAGS += --emit-relocs -Bsymbolic -Bsymbolic-functions

LDFLAGS_FINAL += --gc-sections -pie
LDFLAGS_FINAL += --wrap=__divdi3 --wrap=__udivdi3
LDFLAGS_FINAL += --wrap=__moddi3 --wrap=__umoddi3

export NORMAL_LIBGCC = $(shell $(CC) $(CFLAGS) -print-libgcc-file-name)
PREFIXED_LIBGCC = $(OBJTREE)/arch/$(ARCH)/lib/$(shell basename $(NORMAL_LIBGCC))

export USE_PRIVATE_LIBGCC=$(shell dirname $(PREFIXED_LIBGCC))
