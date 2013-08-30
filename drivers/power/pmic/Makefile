#
# Copyright (C) 2012 Samsung Electronics
# Lukasz Majewski <l.majewski@samsung.com>
#
# SPDX-License-Identifier:	GPL-2.0+
#

include $(TOPDIR)/config.mk

LIB	:= $(obj)libpmic.o

COBJS-$(CONFIG_POWER_MAX8998) += pmic_max8998.o
COBJS-$(CONFIG_POWER_MAX8997) += pmic_max8997.o
COBJS-$(CONFIG_POWER_MUIC_MAX8997) += muic_max8997.o
COBJS-$(CONFIG_POWER_MAX77686) += pmic_max77686.o
COBJS-$(CONFIG_POWER_TPS65217) += pmic_tps65217.o
COBJS-$(CONFIG_POWER_TPS65910) += pmic_tps65910.o

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
