#
# (C) Copyright 2008
# Wolfgang Denk, DENX Software Engineering, wd@denx.de.
#
# SPDX-License-Identifier:	GPL-2.0+
#

include $(TOPDIR)/config.mk

LIB	:= $(obj)libphy.o

COBJS-$(CONFIG_BITBANGMII) += miiphybb.o
COBJS-$(CONFIG_MV88E61XX_SWITCH) += mv88e61xx.o
COBJS-$(CONFIG_MV88E6352_SWITCH) += mv88e6352.o

COBJS-$(CONFIG_PHYLIB) += phy.o
COBJS-$(CONFIG_PHYLIB_10G) += generic_10g.o
COBJS-$(CONFIG_PHY_ATHEROS) += atheros.o
COBJS-$(CONFIG_PHY_BROADCOM) += broadcom.o
COBJS-$(CONFIG_PHY_DAVICOM) += davicom.o
COBJS-$(CONFIG_PHY_ET1011C) += et1011c.o
COBJS-$(CONFIG_PHY_ICPLUS) += icplus.o
COBJS-$(CONFIG_PHY_LXT) += lxt.o
COBJS-$(CONFIG_PHY_MARVELL) += marvell.o
COBJS-$(CONFIG_PHY_MICREL) += micrel.o
COBJS-$(CONFIG_PHY_NATSEMI) += natsemi.o
COBJS-$(CONFIG_PHY_REALTEK) += realtek.o
COBJS-$(CONFIG_PHY_SMSC) += smsc.o
COBJS-$(CONFIG_PHY_TERANETICS) += teranetics.o
COBJS-$(CONFIG_PHY_VITESSE) += vitesse.o

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
