#
# Copyright 2014 Freescale Semiconductor, Inc.
#
# SPDX-License-Identifier:	GPL-2.0+
#

ifdef CONFIG_SPL_BUILD
obj-y += spl.o
else
obj-y	+= t102xqds.o
obj-y	+= eth_t102xqds.o
obj-$(CONFIG_PCI) += pci.o
obj-$(CONFIG_FSL_DIU_FB) += ../t1040qds/diu.o
endif
obj-y   += ddr.o
obj-y   += law.o
obj-y   += tlb.o
