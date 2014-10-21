#
# (C) Copyright 2003
# Wolfgang Denk, DENX Software Engineering, wd@denx.de.
#
# SPDX-License-Identifier:	GPL-2.0+
#

ifeq ($(CROSS_COMPILE),)
CROSS_COMPILE := mips_4KC-
endif

# Handle special prefix in ELDK 4.0 toolchain
ifneq (,$(findstring 4KCle,$(CROSS_COMPILE)))
ENDIANNESS := -EL
endif

ifdef CONFIG_SYS_LITTLE_ENDIAN
ENDIANNESS := -EL
endif

ifdef CONFIG_SYS_BIG_ENDIAN
ENDIANNESS := -EB
endif

# Default to EB if no endianess is configured
ENDIANNESS ?= -EB

PLATFORM_CPPFLAGS += -D__MIPS__

__HAVE_ARCH_GENERIC_BOARD := y

#
# From Linux arch/mips/Makefile
#
# GCC uses -G 0 -mabicalls -fpic as default.  We don't want PIC in the kernel
# code since it only slows down the whole thing.  At some point we might make
# use of global pointer optimizations but their use of $28 conflicts with
# the current pointer optimization.
#
# The DECStation requires an ECOFF kernel for remote booting, other MIPS
# machines may also.  Since BFD is incredibly buggy with respect to
# crossformat linking we rely on the elf2ecoff tool for format conversion.
#
# cflags-y			+= -G 0 -mno-abicalls -fno-pic -pipe
# cflags-y			+= -msoft-float
# LDFLAGS_vmlinux		+= -G 0 -static -n -nostdlib
# MODFLAGS			+= -mlong-calls
#
# On the other hand, we want PIC in the U-Boot code to relocate it from ROM
# to RAM. $28 is always used as gp.
#
PLATFORM_CPPFLAGS		+= -G 0 -mabicalls -fpic $(ENDIANNESS)
PLATFORM_CPPFLAGS		+= -msoft-float
PLATFORM_LDFLAGS		+= -G 0 -static -n -nostdlib $(ENDIANNESS)
PLATFORM_RELFLAGS		+= -ffunction-sections -fdata-sections
LDFLAGS_FINAL			+= --gc-sections -pie
OBJCOPYFLAGS			+= -j .text -j .rodata -j .data -j .got
OBJCOPYFLAGS			+= -j .u_boot_list -j .rel.dyn
