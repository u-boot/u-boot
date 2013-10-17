#
# (C) Copyright 2000-2002
# Wolfgang Denk, DENX Software Engineering, wd@denx.de.
#
# SPDX-License-Identifier:	GPL-2.0+
#

CROSS_COMPILE ?= arm-linux-

ifndef CONFIG_STANDALONE_LOAD_ADDR
ifneq ($(CONFIG_OMAP_COMMON),)
CONFIG_STANDALONE_LOAD_ADDR = 0x80300000
else
CONFIG_STANDALONE_LOAD_ADDR = 0xc100000
endif
endif

LDFLAGS_FINAL += --gc-sections
PLATFORM_RELFLAGS += -ffunction-sections -fdata-sections \
		     -fno-common -ffixed-r9 -msoft-float

# Support generic board on ARM
__HAVE_ARCH_GENERIC_BOARD := y

PLATFORM_CPPFLAGS += -DCONFIG_ARM -D__ARM__

# Choose between ARM/Thumb instruction sets
ifeq ($(CONFIG_SYS_THUMB_BUILD),y)
PF_CPPFLAGS_ARM := $(call cc-option, -mthumb -mthumb-interwork,\
			$(call cc-option,-marm,)\
			$(call cc-option,-mno-thumb-interwork,)\
		)
else
PF_CPPFLAGS_ARM := $(call cc-option,-marm,) \
		$(call cc-option,-mno-thumb-interwork,)
endif

# Only test once
ifneq ($(CONFIG_SPL_BUILD),y)
ALL-$(CONFIG_SYS_THUMB_BUILD)	+= checkthumb
endif

# Try if EABI is supported, else fall back to old API,
# i. e. for example:
# - with ELDK 4.2 (EABI supported), use:
#	-mabi=aapcs-linux
# - with ELDK 4.1 (gcc 4.x, no EABI), use:
#	-mabi=apcs-gnu
# - with ELDK 3.1 (gcc 3.x), use:
#	-mapcs-32
PF_CPPFLAGS_ABI := $(call cc-option,\
			-mabi=aapcs-linux,\
			$(call cc-option,\
				-mapcs-32,\
				$(call cc-option,\
					-mabi=apcs-gnu,\
				)\
			)\
		)
PLATFORM_CPPFLAGS += $(PF_CPPFLAGS_ARM) $(PF_CPPFLAGS_ABI)

# For EABI, make sure to provide raise()
ifneq (,$(findstring -mabi=aapcs-linux,$(PLATFORM_CPPFLAGS)))
# This file is parsed many times, so the string may get added multiple
# times. Also, the prefix needs to be different based on whether
# CONFIG_SPL_BUILD is defined or not. 'filter-out' the existing entry
# before adding the correct one.
ifdef CONFIG_SPL_BUILD
PLATFORM_LIBS := $(SPLTREE)/arch/arm/lib/eabi_compat.o \
	$(filter-out %/arch/arm/lib/eabi_compat.o, $(PLATFORM_LIBS))
else
PLATFORM_LIBS := $(OBJTREE)/arch/arm/lib/eabi_compat.o \
	$(filter-out %/arch/arm/lib/eabi_compat.o, $(PLATFORM_LIBS))
endif
endif

# needed for relocation
LDFLAGS_u-boot += -pie

#
# FIXME: binutils versions < 2.22 have a bug in the assembler where
# branches to weak symbols can be incorrectly optimized in thumb mode
# to a short branch (b.n instruction) that won't reach when the symbol
# gets preempted
#
# http://sourceware.org/bugzilla/show_bug.cgi?id=12532
#
ifeq ($(CONFIG_SYS_THUMB_BUILD),y)
ifeq ($(GAS_BUG_12532),)
export GAS_BUG_12532:=$(shell if [ $(call binutils-version) -lt 0222 ] ; \
	then echo y; else echo n; fi)
endif
ifeq ($(GAS_BUG_12532),y)
PLATFORM_RELFLAGS += -fno-optimize-sibling-calls
endif
endif

ifneq ($(CONFIG_SPL_BUILD),y)
# Check that only R_ARM_RELATIVE relocations are generated.
ALL-y += checkarmreloc
# The movt / movw can hardcode 16 bit parts of the addresses in the
# instruction. Relocation is not supported for that case, so disable
# such usage by requiring word relocations.
PLATFORM_CPPFLAGS += $(call cc-option, -mword-relocations)
endif
