#
# (C) Copyright 2006
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
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program; if not, write to the Free Software
# Foundation, Inc., 59 Temple Place, Suite 330, Boston,
# MA 02111-1307 USA
#

include $(TOPDIR)/config.mk

ifneq ($(OBJTREE),$(SRCTREE))
$(shell mkdir -p $(obj)board/$(VENDOR)/common)
endif

LIB	= $(obj)lib$(VENDOR).o

COBJS-$(CONFIG_FSL_PCI_INIT)	+= fsl_8xxx_pci.o
COBJS-$(CONFIG_MPC8572)		+= fsl_8xxx_clk.o
COBJS-$(CONFIG_MPC86xx)		+= fsl_8xxx_clk.o
COBJS-$(CONFIG_P2020)		+= fsl_8xxx_clk.o
COBJS-$(CONFIG_MPC85xx)		+= fsl_8xxx_misc.o board.o
COBJS-$(CONFIG_MPC86xx)		+= fsl_8xxx_misc.o board.o
COBJS-$(CONFIG_NAND_ACTL)	+= actl_nand.o

SRCS	:= $(SOBJS:.o=.S) $(COBJS-y:.o=.c)
OBJS	:= $(addprefix $(obj),$(COBJS-y))
SOBJS	:= $(addprefix $(obj),$(SOBJS))

$(LIB):	$(obj).depend $(OBJS)
	$(call cmd_link_o_target, $(OBJS))

#########################################################################

# defines $(obj).depend target
include $(SRCTREE)/rules.mk

sinclude $(obj).depend

#########################################################################
