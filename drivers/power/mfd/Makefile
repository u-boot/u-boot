#
# Copyright (C) 2013 Samsung Electronics
# Piotr Wilczek <p.wilczek@samsung.com>
#
# SPDX-License-Identifier:	GPL-2.0+
#

include $(TOPDIR)/config.mk

LIB	:= $(obj)libmfd.o

COBJS-$(CONFIG_POWER_PMIC_MAX77693) += pmic_max77693.o
COBJS-$(CONFIG_POWER_MUIC_MAX77693) += muic_max77693.o
COBJS-$(CONFIG_POWER_FG_MAX77693) += fg_max77693.o

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

########################################################################
