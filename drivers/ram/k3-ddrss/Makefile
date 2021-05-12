# SPDX-License-Identifier: GPL-2.0+
#
# Copyright (C) 2019-2021 Texas Instruments Incorporated - http://www.ti.com/
#

obj-$(CONFIG_K3_DDRSS) += k3-ddrss.o
obj-$(CONFIG_K3_DDRSS) += lpddr4_obj_if.o
obj-$(CONFIG_K3_DDRSS) += lpddr4.o
ccflags-$(CONFIG_K3_DDRSS) += -Idrivers/ram/k3-ddrss/

obj-$(CONFIG_K3_AM64_DDRSS) += lpddr4_16bit.o
obj-$(CONFIG_K3_AM64_DDRSS) += lpddr4_16bit_ctl_regs_rw_masks.o
ccflags-$(CONFIG_K3_AM64_DDRSS) += -Idrivers/ram/k3-ddrss/16bit/

obj-$(CONFIG_K3_J721E_DDRSS) += lpddr4_32bit.o
obj-$(CONFIG_K3_J721E_DDRSS) += lpddr4_32bit_ctl_regs_rw_masks.o
ccflags-$(CONFIG_K3_J721E_DDRSS) += -Idrivers/ram/k3-ddrss/32bit/
