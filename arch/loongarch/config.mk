# SPDX-License-Identifier: GPL-2.0+
#
# Copyright (C) 2024 Jiaxun yang <jiaxun.yang@flygoat.com>
#

32bit-bfd		= elf32-loongarch
64bit-bfd		= elf64-loongarch
32bit-emul		= elf32loongarch
64bit-emul		= elf64loongarch

ifdef CONFIG_32BIT
KBUILD_LDFLAGS		+= -m $(32bit-emul)
PLATFORM_ELFFLAGS	+= -B loongarch -O $(32bit-bfd)
endif

ifdef CONFIG_64BIT
KBUILD_LDFLAGS		+= -m $(64bit-emul)
PLATFORM_ELFFLAGS	+= -B loongarch -O $(64bit-bfd)
endif

PLATFORM_CPPFLAGS	+= -fpic
PLATFORM_RELFLAGS	+= -fno-common -ffunction-sections -fdata-sections
LDFLAGS_u-boot		+= --gc-sections -static -pie
