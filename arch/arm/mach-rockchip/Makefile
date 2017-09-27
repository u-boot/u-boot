#
# Copyright (c) 2014 Google, Inc
#
# SPDX-License-Identifier:	GPL-2.0+
#

# We don't want the bootrom-helper present in a full U-Boot build, as
# this may have entered from ATF with the stack-pointer pointing to
# inaccessible/protected memory (and the bootrom-helper assumes that
# the stack-pointer is valid before switching to the U-Boot stack).
obj-spl-$(CONFIG_ROCKCHIP_BROM_HELPER) += bootrom.o save_boot_param.o
obj-tpl-$(CONFIG_ROCKCHIP_BROM_HELPER) += bootrom.o save_boot_param.o

obj-tpl-$(CONFIG_ROCKCHIP_RK3188) += rk3188-board-tpl.o
obj-tpl-$(CONFIG_ROCKCHIP_RK3288) += rk3288-board-tpl.o
obj-tpl-$(CONFIG_ROCKCHIP_RK3368) += rk3368-board-tpl.o

obj-spl-$(CONFIG_ROCKCHIP_RK3036) += rk3036-board-spl.o
obj-spl-$(CONFIG_ROCKCHIP_RK3188) += rk3188-board-spl.o
obj-spl-$(CONFIG_ROCKCHIP_RK322X) += rk322x-board-spl.o
obj-spl-$(CONFIG_ROCKCHIP_RK3288) += rk3288-board-spl.o
obj-spl-$(CONFIG_ROCKCHIP_RK3368) += rk3368-board-spl.o spl-boot-order.o
obj-spl-$(CONFIG_ROCKCHIP_RK3399) += rk3399-board-spl.o spl-boot-order.o

ifeq ($(CONFIG_SPL_BUILD)$(CONFIG_TPL_BUILD),)
obj-$(CONFIG_ROCKCHIP_RK3188) += rk3188-board.o
obj-$(CONFIG_ROCKCHIP_RK322X) += rk322x-board.o
obj-$(CONFIG_ROCKCHIP_RK3288) += rk3288-board.o
obj-$(CONFIG_ROCKCHIP_RK3036) += rk3036-board.o
endif

obj-$(CONFIG_$(SPL_TPL_)RAM) += sdram_common.o

ifndef CONFIG_ARM64
obj-y += rk_timer.o
endif

obj-$(CONFIG_ROCKCHIP_RK3036) += rk3036/
ifndef CONFIG_TPL_BUILD
obj-$(CONFIG_ROCKCHIP_RK3188) += rk3188/
endif
obj-$(CONFIG_ROCKCHIP_RK322X) += rk322x/
obj-$(CONFIG_ROCKCHIP_RK3288) += rk3288/
obj-$(CONFIG_ROCKCHIP_RK3328) += rk3328/
obj-$(CONFIG_ROCKCHIP_RK3368) += rk3368/
obj-$(CONFIG_ROCKCHIP_RK3399) += rk3399/
obj-$(CONFIG_ROCKCHIP_RV1108) += rv1108/

# Clear out SPL objects, in case this is a TPL build
obj-spl-$(CONFIG_TPL_BUILD) =

# Now add SPL/TPL objects back into the main build
obj-$(CONFIG_SPL_BUILD) += $(obj-spl-y)
obj-$(CONFIG_TPL_BUILD) += $(obj-tpl-y)
