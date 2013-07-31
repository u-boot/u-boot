#
# Copyright 2009-2011 Freescale Semiconductor, Inc.
#
# SPDX-License-Identifier:	GPL-2.0+
#

include $(TOPDIR)/config.mk

LIB	:= $(obj)libfm.o

ifdef CONFIG_FMAN_ENET
COBJS-y += dtsec.o
COBJS-y += eth.o
COBJS-y += fm.o
COBJS-y += init.o
COBJS-y += tgec.o
COBJS-y += tgec_phy.o

# Soc have FMAN v3 with mEMAC
COBJS-$(CONFIG_SYS_FMAN_V3) += memac_phy.o
COBJS-$(CONFIG_SYS_FMAN_V3) += memac.o

# SoC specific SERDES support
COBJS-$(CONFIG_P1017)	+= p1023.o
COBJS-$(CONFIG_P1023)	+= p1023.o
# The P204x, P304x, and P5020 are the same
COBJS-$(CONFIG_PPC_P2041) += p5020.o
COBJS-$(CONFIG_PPC_P3041) += p5020.o
COBJS-$(CONFIG_PPC_P4080) += p4080.o
COBJS-$(CONFIG_PPC_P5020) += p5020.o
COBJS-$(CONFIG_PPC_P5040) += p5040.o
COBJS-$(CONFIG_PPC_T4240) += t4240.o
COBJS-$(CONFIG_PPC_T4160) += t4240.o
COBJS-$(CONFIG_PPC_B4420) += b4860.o
COBJS-$(CONFIG_PPC_B4860) += b4860.o
endif

COBJS	:= $(COBJS-y)
SRCS	:= $(COBJS:.o=.c)
OBJS	:= $(addprefix $(obj),$(COBJS))

all:	$(LIB)

$(LIB):	$(obj).depend $(OBJS)
	$(call cmd_link_o_target, $(OBJS))

#########################################################################

include $(SRCTREE)/rules.mk

sinclude $(obj).depend

#########################################################################
