#
# (C) Copyright 2002-2006
# Wolfgang Denk, DENX Software Engineering, wd@denx.de.
#
# SPDX-License-Identifier:	GPL-2.0+
#
include $(TOPDIR)/config.mk

LIB	= libp_corenet.o

COBJS-y			+= law.o
COBJS-$(CONFIG_PCI)	+= pci.o
COBJS-y			+= tlb.o

include $(TOPDIR)/post/rules.mk
