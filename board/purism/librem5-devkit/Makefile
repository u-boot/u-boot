#
# Copyright 2017 NXP
# Copyright 2026 Phosh.mobi e.V.
#
# SPDX-License-Identifier:      GPL-2.0+
#

obj-y += librem5-devkit.o

ifdef CONFIG_XPL_BUILD
obj-y += spl.o
obj-$(CONFIG_IMX8M_LPDDR4) += lpddr4_timing.o lpddr4_timing_b0.o
endif
