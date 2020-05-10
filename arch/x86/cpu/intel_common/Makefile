# SPDX-License-Identifier: GPL-2.0+
#
# Copyright (c) 2016 Google, Inc

ifdef CONFIG_HAVE_MRC
obj-$(CONFIG_$(SPL_TPL_)X86_16BIT_INIT) += car.o
obj-$(CONFIG_$(SPL_TPL_)X86_32BIT_INIT) += me_status.o
obj-$(CONFIG_$(SPL_TPL_)X86_32BIT_INIT) += report_platform.o
obj-$(CONFIG_$(SPL_TPL_)X86_32BIT_INIT) += mrc.o
endif

ifdef CONFIG_INTEL_CAR_CQOS
obj-$(CONFIG_TPL_BUILD) += car2.o
ifndef CONFIG_SPL_BUILD
obj-y += car2_uninit.o
endif
endif

obj-y += cpu.o
obj-y += fast_spi.o
obj-y += lpc.o
obj-y += lpss.o
ifndef CONFIG_TARGET_EFI_APP
obj-$(CONFIG_$(SPL_TPL_)X86_32BIT_INIT) += microcode.o
ifndef CONFIG_$(SPL_)X86_64
obj-y += microcode.o
endif
endif
obj-y += pch.o
obj-$(CONFIG_HAVE_ITSS) += itss.o
obj-$(CONFIG_HAVE_P2SB) += p2sb.o

ifdef CONFIG_SPL
ifndef CONFIG_SPL_BUILD
ifndef CONFIG_SYS_COREBOOT
obj-y += cpu_from_spl.o
endif
endif
endif
