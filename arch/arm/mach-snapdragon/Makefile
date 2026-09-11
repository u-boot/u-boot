# SPDX-License-Identifier: GPL-2.0+
#
# (C) Copyright 2015 Mateusz Kulikowski <mateusz.kulikowski@gmail.com>

obj-y += dram.o
obj-y += mem_map.o

ifeq ($(CONFIG_SPL_BUILD),y)
obj-y += board_spl.o
else
obj-y += board.o
obj-$(CONFIG_EFI_HAVE_CAPSULE_SUPPORT) += capsule_update.o
endif

obj-$(CONFIG_OF_LIVE) += of_fixup.o
