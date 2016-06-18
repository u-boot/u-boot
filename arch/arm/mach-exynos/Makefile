#
# Copyright (C) 2009 Samsung Electronics
# Minkyu Kang <mk7.kang@samsung.com>
#
# SPDX-License-Identifier:	GPL-2.0+
#

obj-y	+= soc.o
obj-$(CONFIG_CPU_V7) += clock.o pinmux.o power.o system.o
obj-$(CONFIG_ARM64)	+= mmu-arm64.o

obj-$(CONFIG_EXYNOS5420)	+= sec_boot.o

ifdef CONFIG_SPL_BUILD
obj-$(CONFIG_EXYNOS5)	+= clock_init_exynos5.o
obj-$(CONFIG_EXYNOS5)	+= dmc_common.o dmc_init_ddr3.o
obj-$(CONFIG_EXYNOS4210)+= dmc_init_exynos4.o clock_init_exynos4.o
obj-y	+= spl_boot.o tzpc.o
obj-y	+= lowlevel_init.o
endif
