#
# (C) Copyright 2000-2007
# Wolfgang Denk, DENX Software Engineering, wd@denx.de.
#
# SPDX-License-Identifier:	GPL-2.0+
#

include $(TOPDIR)/config.mk

LIB	:= $(obj)libpci.o

COBJS-$(CONFIG_FSL_PCI_INIT) += fsl_pci_init.o
COBJS-$(CONFIG_PCI) += pci.o pci_auto.o
COBJS-$(CONFIG_PCI_INDIRECT_BRIDGE) += pci_indirect.o
COBJS-$(CONFIG_PCI_GT64120) += pci_gt64120.o
COBJS-$(CONFIG_FTPCI100) += pci_ftpci100.o
COBJS-$(CONFIG_IXP_PCI) += pci_ixp.o
COBJS-$(CONFIG_SH4_PCI) += pci_sh4.o
COBJS-$(CONFIG_SH7751_PCI) +=pci_sh7751.o
COBJS-$(CONFIG_SH7780_PCI) +=pci_sh7780.o
COBJS-$(CONFIG_TSI108_PCI) += tsi108_pci.o
COBJS-$(CONFIG_WINBOND_83C553) += w83c553f.o

COBJS	:= $(COBJS-y)
SRCS	:= $(COBJS:.o=.c)
OBJS	:= $(addprefix $(obj),$(COBJS))

all:	$(LIB)

$(LIB):	$(obj).depend $(OBJS)
	$(call cmd_link_o_target, $(OBJS))

#########################################################################

# defines $(obj).depend target
include $(SRCTREE)/rules.mk

sinclude $(obj).depend

#########################################################################
