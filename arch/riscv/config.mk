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

PLATFORM_CPPFLAGS	+= -ffixed-x3 -fpic
PLATFORM_RELFLAGS	+= -fno-common -ffunction-sections -fdata-sections
LDFLAGS_u-boot		+= --gc-sections -static -pie

EFI_CRT0		:= crt0_riscv_efi.o
EFI_RELOC		:= reloc_riscv_efi.o
