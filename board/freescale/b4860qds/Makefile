#
# Copyright 2012 Freescale Semiconductor, Inc.
#
# SPDX-License-Identifier:	GPL-2.0+
#

ifdef CONFIG_SPL_BUILD
obj-y	+= spl.o
else
obj-y	+= b4860qds.o
obj-$(CONFIG_B4860QDS)	+= eth_b4860qds.o
obj-$(CONFIG_PCI)	+= pci.o
endif

obj-y	+= ddr.o
obj-y	+= law.o
obj-y	+= tlb.o
