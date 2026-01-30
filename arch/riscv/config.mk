# SPDX-License-Identifier: GPL-2.0+
#
# (C) Copyright 2000-2002
# Wolfgang Denk, DENX Software Engineering, wd@denx.de.
#
# Copyright (c) 2017 Microsemi Corporation.
# Padmarao Begari, Microsemi Corporation <padmarao.begari@microsemi.com>
#
# Copyright (C) 2017 Andes Technology Corporation
# Rick Chen, Andes Technology Corporation <rick@andestech.com>
#

ifdef CONFIG_SYS_BIG_ENDIAN
small-endian		:= b
large-endian		:= big
PLATFORM_CPPFLAGS       += -mbig-endian
KBUILD_LDFLAGS          += -mbig-endian
else
small-endian		:= l
large-endian		:= little
endif

32bit-emul		:= elf32$(small-endian)riscv
64bit-emul		:= elf64$(small-endian)riscv

ifdef CONFIG_32BIT
KBUILD_LDFLAGS		+= -m $(32bit-emul)
EFI_LDS			:= elf_riscv32_efi.lds
PLATFORM_ELFFLAGS	+= -B riscv -O elf32-$(large-endian)riscv
endif

ifdef CONFIG_64BIT
KBUILD_LDFLAGS		+= -m $(64bit-emul)
EFI_LDS			:= elf_riscv64_efi.lds
PLATFORM_ELFFLAGS	+= -B riscv -O elf64-$(large-endian)riscv
endif

PLATFORM_CPPFLAGS	+= -ffixed-x3
PLATFORM_RELFLAGS	+= -fno-common -ffunction-sections -fdata-sections
LDFLAGS_u-boot		+= --gc-sections -static

#
# Build PIE/PIC only when requested. Bare-metal toolchains (riscv*-unknown-elf)
# commonly do not support -pie, and many embedded targets (e.g. NEORV32 bring-up)
# want a fixed-address static image.
#
ifdef CONFIG_POSITION_INDEPENDENT
PLATFORM_CPPFLAGS       += -fpic
LDFLAGS_u-boot          += -pie
endif

#
# NEORV32 bring-up: toolchain is rv32i/ilp32 (no FPU). U-Boot defaults may add
# rv32imafdc/ilp32d which breaks linking with a soft-float libgcc.
# Force -march/-mabi for NEORV32 and strip any previous ones to avoid duplicates.
#
ifeq ($(CONFIG_TARGET_NEORV32),y)
# Strip any earlier -march/-mabi that might have been injected by Kconfig/Makefiles
PLATFORM_CPPFLAGS := $(filter-out -march=% -mabi=%,$(PLATFORM_CPPFLAGS))
PLATFORM_RELFLAGS := $(filter-out -march=% -mabi=%,$(PLATFORM_RELFLAGS))
KBUILD_CFLAGS     := $(filter-out -march=% -mabi=%,$(KBUILD_CFLAGS))
KBUILD_AFLAGS     := $(filter-out -march=% -mabi=%,$(KBUILD_AFLAGS))

# IMPORTANT: ld is called directly, so DO NOT pass -march/-mabi via any LDFLAGS
KBUILD_LDFLAGS    := $(filter-out -march=% -mabi=%,$(KBUILD_LDFLAGS))
LDFLAGS_u-boot    := $(filter-out -march=% -mabi=%,$(LDFLAGS_u-boot))

# Add the correct soft-float ISA/ABI (compiler/assembler only)
NEORV32_MARCH := rv32i_zicsr_zifencei
NEORV32_MABI  := ilp32

PLATFORM_CPPFLAGS += -march=$(NEORV32_MARCH) -mabi=$(NEORV32_MABI)
PLATFORM_RELFLAGS += -march=$(NEORV32_MARCH) -mabi=$(NEORV32_MABI)
KBUILD_CFLAGS     += -march=$(NEORV32_MARCH) -mabi=$(NEORV32_MABI)
KBUILD_AFLAGS     += -march=$(NEORV32_MARCH) -mabi=$(NEORV32_MABI)
endif

EFI_CRT0		:= crt0_riscv_efi.o
EFI_RELOC		:= reloc_riscv_efi.o

