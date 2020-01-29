#
# Copyright 2018 NXP
#
# SPDX-License-Identifier:	GPL-2.0+
#

obj-y += cpu.o iomux.o misc.o lowlevel_init.o
obj-$(CONFIG_OF_SYSTEM_SETUP) += fdt.o

ifdef CONFIG_SPL_BUILD
obj-$(CONFIG_SPL_LOAD_IMX_CONTAINER) += image.o parse-container.o
endif
