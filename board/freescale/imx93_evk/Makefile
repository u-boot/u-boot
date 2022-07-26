#
# Copyright 2022 NXP
#
# SPDX-License-Identifier:      GPL-2.0+
#

obj-y += imx93_evk.o

ifdef CONFIG_SPL_BUILD
obj-y += spl.o
obj-$(CONFIG_IMX93_EVK_LPDDR4X) += lpddr4x_timing.o
endif
