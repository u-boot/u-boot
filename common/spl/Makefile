#
# (C) Copyright 2012
# Texas Instruments Incorporated - http://www.ti.com/
# Aneesh V <aneesh@ti.com>
#
# SPDX-License-Identifier:	GPL-2.0+
#
# Based on common/Makefile.
#

include $(TOPDIR)/config.mk

LIB	= $(obj)libspl.o

ifdef CONFIG_SPL_BUILD
COBJS-$(CONFIG_SPL_FRAMEWORK) += spl.o
COBJS-$(CONFIG_SPL_NOR_SUPPORT) += spl_nor.o
COBJS-$(CONFIG_SPL_YMODEM_SUPPORT) += spl_ymodem.o
COBJS-$(CONFIG_SPL_NAND_SUPPORT) += spl_nand.o
COBJS-$(CONFIG_SPL_ONENAND_SUPPORT) += spl_onenand.o
COBJS-$(CONFIG_SPL_NET_SUPPORT) += spl_net.o
COBJS-$(CONFIG_SPL_MMC_SUPPORT) += spl_mmc.o
endif

COBJS	:= $(sort $(COBJS-y))
SRCS	:= $(COBJS:.o=.c)
OBJS	:= $(addprefix $(obj),$(COBJS-y))

all:	$(obj).depend $(LIB)

$(LIB):	$(OBJS)
	$(call cmd_link_o_target, $(OBJS))

#########################################################################

# defines $(obj).depend target
include $(SRCTREE)/rules.mk

sinclude $(obj).depend

#########################################################################
