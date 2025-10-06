# SPDX-License-Identifier: GPL-2.0-or-later
#
# Copyright (C) 2025 PHYTEC Messtechnik GmbH

obj-y += imx8mp-libra-fpsc.o

ifdef CONFIG_XPL_BUILD
obj-y += spl.o
obj-$(CONFIG_IMX8M_LPDDR4) += lpddr4_timing.o
endif
