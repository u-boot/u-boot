#
# (C) Copyright 2003
# Wolfgang Denk, DENX Software Engineering, wd@denx.de.
#
# SPDX-License-Identifier:	GPL-2.0+
#

ifdef CONFIG_SYS_BIG_ENDIAN
32bit-emul		:= elf32btsmip
64bit-emul		:= elf64btsmip
32bit-bfd		:= elf32-tradbigmips
64bit-bfd		:= elf64-tradbigmips
PLATFORM_CPPFLAGS	+= -EB
PLATFORM_LDFLAGS	+= -EB
endif

ifdef CONFIG_SYS_LITTLE_ENDIAN
32bit-emul		:= elf32ltsmip
64bit-emul		:= elf64ltsmip
32bit-bfd		:= elf32-tradlittlemips
64bit-bfd		:= elf64-tradlittlemips
PLATFORM_CPPFLAGS	+= -EL
PLATFORM_LDFLAGS	+= -EL
endif

ifdef CONFIG_32BIT
PLATFORM_CPPFLAGS	+= -mabi=32
PLATFORM_LDFLAGS	+= -m $(32bit-emul)
OBJCOPYFLAGS		+= -O $(32bit-bfd)
endif

ifdef CONFIG_64BIT
PLATFORM_CPPFLAGS	+= -mabi=64
PLATFORM_LDFLAGS	+= -m$(64bit-emul)
OBJCOPYFLAGS		+= -O $(64bit-bfd)
endif

cpuflags-$(CONFIG_CPU_MIPS32_R1) += -march=mips32 -Wa,-mips32
cpuflags-$(CONFIG_CPU_MIPS32_R2) += -march=mips32r2 -Wa,-mips32r2
cpuflags-$(CONFIG_CPU_MIPS64_R1) += -march=mips64 -Wa,-mips64
cpuflags-$(CONFIG_CPU_MIPS64_R2) += -march=mips64r2 -Wa,-mips64r2
PLATFORM_CPPFLAGS += $(cpuflags-y)

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
PLATFORM_CPPFLAGS		+= -G 0 -mabicalls -fpic
PLATFORM_CPPFLAGS		+= -msoft-float
PLATFORM_LDFLAGS		+= -G 0 -static -n -nostdlib
PLATFORM_RELFLAGS		+= -ffunction-sections -fdata-sections
LDFLAGS_FINAL			+= --gc-sections -pie
OBJCOPYFLAGS			+= -j .text -j .rodata -j .data -j .got
OBJCOPYFLAGS			+= -j .u_boot_list -j .rel.dyn -j .padding
