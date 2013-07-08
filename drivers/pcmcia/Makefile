#
# (C) Copyright 2000-2007
# Wolfgang Denk, DENX Software Engineering, wd@denx.de.
#
# SPDX-License-Identifier:	GPL-2.0+
#

include $(TOPDIR)/config.mk

LIB	:= $(obj)libpcmcia.o

COBJS-$(CONFIG_I82365) += i82365.o
COBJS-$(CONFIG_8xx) += mpc8xx_pcmcia.o
COBJS-y += rpx_pcmcia.o
COBJS-$(CONFIG_IDE_TI_CARDBUS) += ti_pci1410a.o
COBJS-y += tqm8xx_pcmcia.o
COBJS-$(CONFIG_MARUBUN_PCCARD) += marubun_pcmcia.o

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
