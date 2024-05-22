# SPDX-License-Identifier: GPL-2.0+
#
# Copyright (C) 2024 Jiaxun yang <jiaxun.yang@flygoat.com>
#

ARCH_FLAGS = -march=loongarch64 -mabi=lp64s -msoft-float

ifeq ($(CONFIG_$(SPL_)FRAMEPOINTER),y)
	ARCH_FLAGS += -fno-omit-frame-pointer
endif

PLATFORM_CPPFLAGS += $(ARCH_FLAGS)

head-y := arch/loongarch/cpu/start.o

libs-y += arch/loongarch/cpu/
libs-y += arch/loongarch/cpu/$(CPU)/
libs-y += arch/loongarch/lib/

