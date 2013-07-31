#
# (C) Copyright 2008 Stefan Roese <sr@denx.de>, DENX Software Engineering
#
# SPDX-License-Identifier:	GPL-2.0+
#

include $(TOPDIR)/config.mk

LIB	= $(obj)lib$(BOARD).o

COBJS-y := $(BOARD).o
COBJS-y += ebi.o
COBJS-$(CONFIG_VCT_NOR) += ebi_nor_flash.o
COBJS-$(CONFIG_VCT_ONENAND) += ebi_onenand.o
COBJS-$(CONFIG_DRIVER_SMC911X) += ebi_smc911x.o smc_eeprom.o
COBJS-y += gpio.o
COBJS-y += top.o
COBJS-$(CONFIG_USB_EHCI_VCT) += dcgu.o ehci.o scc.o

COBJS	:= $(sort $(COBJS-y))
SRCS	:= $(SOBJS:.o=.S) $(COBJS:.o=.c)
OBJS	:= $(addprefix $(obj),$(COBJS))
SOBJS	:= $(addprefix $(obj),$(SOBJS))

$(LIB):	$(OBJS) $(SOBJS)
	$(call cmd_link_o_target, $(OBJS) $(SOBJS))

#########################################################################

# defines $(obj).depend target
include $(SRCTREE)/rules.mk

sinclude $(obj).depend

#########################################################################
