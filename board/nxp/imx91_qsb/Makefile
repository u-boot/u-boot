#
# Copyright 2024 NXP
#
# SPDX-License-Identifier:      GPL-2.0+
#

obj-y += imx91_qsb.o

ifdef CONFIG_SPL_BUILD
obj-y += spl.o
ifdef CONFIG_IMX9_DRAM_INLINE_ECC
obj-$(CONFIG_TARGET_IMX91_9X9_QSB) += lpddr4_timing_ecc.o lpddr4_timing_1600mts_ecc.o
else
obj-$(CONFIG_TARGET_IMX91_9X9_QSB) += lpddr4_timing.o lpddr4_timing_1600mts.o
endif
endif
