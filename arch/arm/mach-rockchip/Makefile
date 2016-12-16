#
# Copyright (c) 2014 Google, Inc
#
# SPDX-License-Identifier:	GPL-2.0+
#

ifdef CONFIG_SPL_BUILD
obj-$(CONFIG_ROCKCHIP_RK3036) += rk3036-board-spl.o
obj-$(CONFIG_ROCKCHIP_RK3288) += rk3288-board-spl.o
obj-$(CONFIG_ROCKCHIP_SPL_BACK_TO_BROM) += save_boot_param.o
else
obj-$(CONFIG_ROCKCHIP_RK3288) += rk3288-board.o
obj-$(CONFIG_ROCKCHIP_RK3036) += rk3036-board.o
endif
ifndef CONFIG_ARM64
obj-y += rk_timer.o
endif
obj-$(CONFIG_ROCKCHIP_RK3036) += rk3036/
obj-$(CONFIG_ROCKCHIP_RK3288) += rk3288/
obj-$(CONFIG_ROCKCHIP_RK3399) += rk3399/
