# SPDX-License-Identifier: GPL-2.0+
#
# Copyright (C) 2023 Texas Instruments Incorporated - https://www.ti.com/
#	Andrew Davis <afd@ti.com>

obj-$(CONFIG_SOC_K3_AM625) += am62x/
obj-$(CONFIG_SOC_K3_AM62A7) += am62ax/
obj-$(CONFIG_SOC_K3_AM62P5) += am62px/
obj-$(CONFIG_SOC_K3_J721E) += j721e/
obj-$(CONFIG_SOC_K3_J7200) += j7200/
obj-$(CONFIG_SOC_K3_J721S2) += j721s2/
obj-$(CONFIG_SOC_K3_J722S) += j722s/
obj-$(CONFIG_SOC_K3_J784S4) += j784s4/

obj-y += common.o
obj-y += lowlevel_init.o
obj-y += r5_mpu.o

ifeq ($(CONFIG_XPL_BUILD),y)
obj-$(CONFIG_K3_LOAD_SYSFW) += sysfw-loader.o
endif
