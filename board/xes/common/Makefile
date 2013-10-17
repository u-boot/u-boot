#
# (C) Copyright 2006
# Wolfgang Denk, DENX Software Engineering, wd@denx.de.
#
# SPDX-License-Identifier:	GPL-2.0+
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
