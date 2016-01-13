#
# Copyright (c) 2014 Google, Inc
#
# SPDX-License-Identifier:	GPL-2.0+
#

ifdef CONFIG_SPL_BUILD
obj-$(CONFIG_ROCKCHIP_RK3288) += rk3288-board-spl.o
obj-$(CONFIG_ROCKCHIP_RK3036) += rk3036-board-spl.o
else
obj-$(CONFIG_ROCKCHIP_RK3288) += board.o
endif
obj-y += rk_timer.o
obj-y += rk_early_print.o
obj-$(CONFIG_$(SPL_)ROCKCHIP_COMMON) += common.o
obj-$(CONFIG_ROCKCHIP_RK3288) += rk3288/
obj-$(CONFIG_ROCKCHIP_RK3036) += rk3036/
