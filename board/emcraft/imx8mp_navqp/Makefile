#
# Copyright 2019 NXP
# Copyright 2024 Gilles Talis <gilles.talis@gmail.com>
#
# SPDX-License-Identifier:      GPL-2.0+
#

obj-y += imx8mp_navqp.o

ifdef CONFIG_SPL_BUILD
obj-y += spl.o
obj-$(CONFIG_IMX8M_LPDDR4) += lpddr4_timing.o
endif
