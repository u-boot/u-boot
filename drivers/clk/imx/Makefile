# Copyright 2018 NXP
#
# SPDX-License-Identifier: GPL-2.0

obj-$(CONFIG_$(SPL_TPL_)CLK_CCF) += clk-gate2.o clk-pllv3.o clk-pfd.o
obj-$(CONFIG_$(SPL_TPL_)CLK_IMX6Q) += clk-imx6q.o
obj-$(CONFIG_CLK_IMX8) += clk-imx8.o

ifdef CONFIG_CLK_IMX8
obj-$(CONFIG_IMX8QXP) += clk-imx8qxp.o
obj-$(CONFIG_IMX8QM) += clk-imx8qm.o
endif
