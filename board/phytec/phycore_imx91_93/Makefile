#
# Copyright 2022 NXP
# Copyright (C) 2023 PHYTEC Messtechnik GmbH
# Christoph Stoidner <c.stoidner@phytec.de>
# Copyright (C) 2024 Mathieu Othacehe <m.othacehe@gmail.com>
#
# SPDX-License-Identifier:      GPL-2.0+
#

obj-y += phycore-imx91-93.o

ifdef CONFIG_XPL_BUILD
obj-y += spl.o
ifdef CONFIG_IMX91
obj-$(CONFIG_IMX9_LPDDR4X) += lpddr4_timing_imx91.o
else
obj-$(CONFIG_IMX9_LPDDR4X) += lpddr4_timing_imx93.o
endif
endif
