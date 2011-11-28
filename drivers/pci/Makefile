#
# (C) Copyright 2000-2007
# Wolfgang Denk, DENX Software Engineering, wd@denx.de.
#
# See file CREDITS for list of people who contributed to this
# project.
#
# This program is free software; you can redistribute it and/or
# modify it under the terms of the GNU General Public License as
# published by the Free Software Foundation; either version 2 of
# the License, or (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program; if not, write to the Free Software
# Foundation, Inc., 59 Temple Place, Suite 330, Boston,
# MA 02111-1307 USA
#

include $(TOPDIR)/config.mk

LIB	:= $(obj)libpci.o

COBJS-$(CONFIG_FSL_PCI_INIT) += fsl_pci_init.o
COBJS-$(CONFIG_PCI) += pci.o pci_auto.o pci_indirect.o
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
