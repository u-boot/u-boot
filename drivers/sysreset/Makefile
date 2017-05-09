#
# (C) Copyright 2016 Cadence Design Systems Inc.
#
# SPDX-License-Identifier:	GPL-2.0+
#

obj-$(CONFIG_SYSRESET) += sysreset-uclass.o
obj-$(CONFIG_SYSRESET_PSCI) += sysreset_psci.o

ifndef CONFIG_SPL_BUILD
obj-$(CONFIG_ROCKCHIP_RK3036) += sysreset_rk3036.o
endif
obj-$(CONFIG_ROCKCHIP_RK3188) += sysreset_rk3188.o
obj-$(CONFIG_ROCKCHIP_RK3288) += sysreset_rk3288.o
obj-$(CONFIG_ROCKCHIP_RK3328) += sysreset_rk3328.o
obj-$(CONFIG_ROCKCHIP_RK3399) += sysreset_rk3399.o
obj-$(CONFIG_SANDBOX) += sysreset_sandbox.o
obj-$(CONFIG_ARCH_SNAPDRAGON) += sysreset_snapdragon.o
obj-$(CONFIG_ARCH_STI) += sysreset_sti.o
obj-$(CONFIG_TARGET_XTFPGA) += sysreset_xtfpga.o
obj-$(CONFIG_ARCH_ASPEED) += sysreset_ast.o
