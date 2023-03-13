# SPDX-License-Identifier: GPL-2.0+
#
# (C) Copyright 2000-2007
# Wolfgang Denk, DENX Software Engineering, wd@denx.de.

include $(srctree)/scripts/dtc/libfdt/Makefile.libfdt
ifneq ($(CONFIG_OF_LIBFDT_OVERLAY),y)
LIBFDT_OBJS := $(filter-out fdt_overlay.o,$(LIBFDT_OBJS))
endif

obj-y += $(LIBFDT_OBJS)

ccflags-y := -I$(srctree)/scripts/dtc/libfdt \
	-DFDT_ASSUME_MASK=$(CONFIG_$(SPL_TPL_)OF_LIBFDT_ASSUME_MASK)
