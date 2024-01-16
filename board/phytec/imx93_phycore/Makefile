#
# Copyright 2022 NXP
# Copyright (C) 2023 PHYTEC Messtechnik GmbH
# Christoph Stoidner <c.stoidner@phytec.de>
#
# SPDX-License-Identifier:      GPL-2.0+
#

obj-y += imx93_phycore.o

ifdef CONFIG_SPL_BUILD
obj-y += spl.o
obj-$(CONFIG_IMX93_PHYCORE_LPDDR4X_1GB) += lpddr4_timing_1GB.o
else
obj-y += ../common/mmc.o
endif
