#
# Copyright 2021 Gateworks Corporation
#
# SPDX-License-Identifier: GPL-2.0+
#

obj-y += imx8mm_venice.o gsc.o

ifdef CONFIG_SPL_BUILD
obj-y += spl.o
obj-$(CONFIG_IMX8M_LPDDR4) += lpddr4_timing.o
endif
