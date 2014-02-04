#
# (C) Copyright 2002-2006
# Wolfgang Denk, DENX Software Engineering, wd@denx.de.
#
# SPDX-License-Identifier:	GPL-2.0+
#

# Compile for a hosted environment on the target
HOST_EXTRACFLAGS  = -idirafter $(SRCTREE)/include \
		-idirafter $(SRCTREE)/arch/$(ARCH)/include \
		-idirafter $(OBJTREE)/include \
		-idirafter $(SRCTREE)/tools/env \
		-DUSE_HOSTCC \
		-DTEXT_BASE=$(TEXT_BASE)

ifeq ($(MTD_VERSION),old)
HOST_EXTRACFLAGS += -DMTD_OLD
endif

hostprogs-y := fw_printenv
always := $(hostprogs-y)

fw_printenv-objs := fw_env.o fw_env_main.o \
	crc32.o ctype.o linux_string.o \
	env_attr.o env_flags.o
