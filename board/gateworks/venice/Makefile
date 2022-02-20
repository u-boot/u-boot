#
# Copyright 2021 Gateworks Corporation
#
# SPDX-License-Identifier: GPL-2.0+
#

obj-y += venice.o gsc.o

ifdef CONFIG_SPL_BUILD
obj-y += spl.o
ifdef CONFIG_IMX8MM
obj-$(CONFIG_IMX8M_LPDDR4) += lpddr4_timing_imx8mm.o
endif
ifdef CONFIG_IMX8MN
obj-$(CONFIG_IMX8M_LPDDR4) += lpddr4_timing_imx8mn.o
endif
endif
