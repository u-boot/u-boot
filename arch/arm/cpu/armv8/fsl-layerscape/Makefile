#
# Copyright 2014-2015, Freescale Semiconductor
#
# SPDX-License-Identifier:	GPL-2.0+
#

obj-y += cpu.o
obj-y += lowlevel.o
obj-y += soc.o
obj-$(CONFIG_MP) += mp.o
obj-$(CONFIG_OF_LIBFDT) += fdt.o
obj-$(CONFIG_SPL) += spl.o
obj-$(CONFIG_FSL_LS_PPA) += ppa.o

ifneq ($(CONFIG_FSL_LSCH3),)
obj-y += fsl_lsch3_speed.o
obj-$(CONFIG_SYS_HAS_SERDES) += fsl_lsch3_serdes.o
else
ifneq ($(CONFIG_FSL_LSCH2),)
obj-y += fsl_lsch2_speed.o
obj-$(CONFIG_SYS_HAS_SERDES) += fsl_lsch2_serdes.o
endif
endif

ifneq ($(CONFIG_LS2080A),)
obj-$(CONFIG_SYS_HAS_SERDES) += ls2080a_serdes.o
endif

ifneq ($(CONFIG_LS1043A),)
obj-$(CONFIG_SYS_HAS_SERDES) += ls1043a_serdes.o
endif

ifneq ($(CONFIG_LS1012A),)
obj-$(CONFIG_SYS_HAS_SERDES) += ls1012a_serdes.o
endif

ifneq ($(CONFIG_LS1046A),)
obj-$(CONFIG_SYS_HAS_SERDES) += ls1046a_serdes.o
endif
