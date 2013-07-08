#
# Copyright (C) 2005-2007 Samsung Electronics.
# Kyungmin Park <kyungmin.park@samsung.com>
#
# SPDX-License-Identifier:	GPL-2.0+
#

include $(TOPDIR)/config.mk

LIB	:= $(obj)libonenand.o

ifndef	CONFIG_SPL_BUILD
COBJS-$(CONFIG_CMD_ONENAND)	:= onenand_uboot.o onenand_base.o onenand_bbt.o
COBJS-$(CONFIG_SAMSUNG_ONENAND)	+= samsung.o
else
COBJS-y				:= onenand_spl.o
endif

COBJS	:= $(COBJS-y)
SRCS	:= $(COBJS:.o=.c)
OBJS	:= $(addprefix $(obj),$(COBJS))

all:	$(LIB)

$(LIB): $(obj).depend $(OBJS)
	$(call cmd_link_o_target, $(OBJS))

#########################################################################

include $(SRCTREE)/rules.mk

sinclude $(obj).depend

#########################################################################
