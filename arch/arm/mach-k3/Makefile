# SPDX-License-Identifier:	GPL-2.0+
#
# Copyright (C) 2017-2018 Texas Instruments Incorporated - http://www.ti.com/
#	Lokesh Vutla <lokeshvutla@ti.com>

obj-$(CONFIG_SOC_K3_AM6) += am6_init.o
obj-$(CONFIG_ARM64) += arm64-mmu.o
obj-$(CONFIG_CPU_V7R) += r5_mpu.o lowlevel_init.o
obj-$(CONFIG_TI_SECURE_DEVICE) += security.o
ifeq ($(CONFIG_SPL_BUILD),y)
obj-$(CONFIG_K3_LOAD_SYSFW) += sysfw-loader.o
endif
obj-y += common.o
