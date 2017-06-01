#
# Copyright (c) 2014 Google, Inc
#
# SPDX-License-Identifier:	GPL-2.0+
#

obj-$(CONFIG_ROCKCHIP_BROM_HELPER) += bootrom.o

ifdef CONFIG_TPL_BUILD
obj-$(CONFIG_ROCKCHIP_RK3188) += rk3188-board-tpl.o
obj-$(CONFIG_ROCKCHIP_BROM_HELPER) += save_boot_param.o
else ifdef CONFIG_SPL_BUILD
obj-$(CONFIG_ROCKCHIP_RK3036) += rk3036-board-spl.o
obj-$(CONFIG_ROCKCHIP_RK3188) += rk3188-board-spl.o
obj-$(CONFIG_ROCKCHIP_RK3288) += rk3288-board-spl.o
obj-$(CONFIG_ROCKCHIP_RK3399) += rk3399-board-spl.o
obj-$(CONFIG_ROCKCHIP_BROM_HELPER) += save_boot_param.o
else
obj-$(CONFIG_ROCKCHIP_RK3188) += rk3188-board.o
obj-$(CONFIG_ROCKCHIP_RK3288) += rk3288-board.o
obj-$(CONFIG_ROCKCHIP_RK3036) += rk3036-board.o
endif
ifndef CONFIG_ARM64
obj-y += rk_timer.o
endif
obj-$(CONFIG_ROCKCHIP_RK3036) += rk3036/

ifndef CONFIG_TPL_BUILD
obj-$(CONFIG_ROCKCHIP_RK3188) += rk3188/
endif

obj-$(CONFIG_ROCKCHIP_RK3288) += rk3288/
obj-$(CONFIG_ROCKCHIP_RK3328) += rk3328/
obj-$(CONFIG_ROCKCHIP_RK3368) += rk3368/
obj-$(CONFIG_ROCKCHIP_RK3399) += rk3399/
obj-$(CONFIG_ROCKCHIP_RV1108) += rv1108/
