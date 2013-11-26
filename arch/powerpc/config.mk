#
# (C) Copyright 2000-2010
# Wolfgang Denk, DENX Software Engineering, wd@denx.de.
#
# SPDX-License-Identifier:	GPL-2.0+
#

CROSS_COMPILE ?= ppc_8xx-

CONFIG_STANDALONE_LOAD_ADDR ?= 0x40000
LDFLAGS_FINAL += --gc-sections
PLATFORM_RELFLAGS += -fpic -mrelocatable -ffunction-sections -fdata-sections \
								-meabi
PLATFORM_CPPFLAGS += -DCONFIG_PPC -D__powerpc__ -ffixed-r2
PLATFORM_LDFLAGS  += -n

# Support generic board on PPC
__HAVE_ARCH_GENERIC_BOARD := y

#
# When cross-compiling on NetBSD, we have to define __PPC__ or else we
# will pick up a va_list declaration that is incompatible with the
# actual argument lists emitted by the compiler.
#
# [Tested on NetBSD/i386 1.5 + cross-powerpc-netbsd-1.3]

ifeq ($(CROSS_COMPILE),powerpc-netbsd-)
PLATFORM_CPPFLAGS+= -D__PPC__
endif
ifeq ($(CROSS_COMPILE),powerpc-openbsd-)
PLATFORM_CPPFLAGS+= -D__PPC__
endif

# Only test once
ifneq ($(CONFIG_SPL_BUILD),y)
ALL-y += checkgcc4
endif
