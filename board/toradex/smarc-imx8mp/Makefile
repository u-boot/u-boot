# SPDX-License-Identifier: GPL-2.0-or-later
# Copyright (C) 2024 Toradex

obj-y += smarc-imx8mp.o

ifdef CONFIG_SPL_BUILD
obj-y += spl.o
obj-$(CONFIG_IMX8M_LPDDR4) += lpddr4_timing.o
endif
