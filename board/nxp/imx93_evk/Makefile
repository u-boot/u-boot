#
# Copyright 2022 NXP
#
# SPDX-License-Identifier:      GPL-2.0+
#

obj-y += imx93_evk.o

ifdef CONFIG_XPL_BUILD
obj-y += spl.o
ifdef CONFIG_IMX9_DRAM_INLINE_ECC
ifdef CONFIG_TARGET_IMX93W_EVK
obj-$(CONFIG_IMX93_EVK_LPDDR4X) += lpddr4x_timing_3600mts.o lpddr4x_timing_1866mts.o
else
obj-$(CONFIG_IMX93_EVK_LPDDR4X) += lpddr4x_timing.o lpddr4x_timing_1866mts.o
endif
else
ifdef CONFIG_TARGET_IMX93W_EVK
obj-$(CONFIG_IMX93_EVK_LPDDR4X) += lpddr4x_timing_3600mts.o lpddr4x_timing_1866mts.o
else
obj-$(CONFIG_IMX93_EVK_LPDDR4X) += lpddr4x_timing.o lpddr4x_timing_1866mts.o
endif
endif
endif
