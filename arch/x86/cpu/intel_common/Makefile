# SPDX-License-Identifier: GPL-2.0+
#
# Copyright (c) 2016 Google, Inc

ifdef CONFIG_HAVE_MRC
obj-$(CONFIG_$(SPL_)X86_32BIT_INIT) += car.o
obj-$(CONFIG_$(SPL_)X86_32BIT_INIT) += me_status.o
obj-$(CONFIG_$(SPL_)X86_32BIT_INIT) += report_platform.o
obj-$(CONFIG_$(SPL_)X86_32BIT_INIT) += mrc.o
endif
obj-y += cpu.o
obj-y += lpc.o
ifndef CONFIG_TARGET_EFI_APP
obj-y += microcode.o
endif
obj-y += pch.o
