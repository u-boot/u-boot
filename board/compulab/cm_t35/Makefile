#
# (C) Copyright 2011 - 2013 CompuLab, Ltd. <www.compulab.co.il>
#
# Authors: Nikita Kiryanov <nikita@compulab.co.il>
#	   Igor Grinberg <grinberg@compulab.co.il>
#
# SPDX-License-Identifier:	GPL-2.0+
#

include $(TOPDIR)/config.mk

LIB	= $(obj)lib$(BOARD).o

COBJS-$(CONFIG_DRIVER_OMAP34XX_I2C) += eeprom.o
COBJS-$(CONFIG_LCD) += display.o

COBJS	:= cm_t35.o leds.o $(COBJS-y)

SRCS	:= $(COBJS:.o=.c)
OBJS	:= $(addprefix $(obj),$(COBJS))

$(LIB):	$(obj).depend $(OBJS)
	$(call cmd_link_o_target, $(OBJS))

#########################################################################

# defines $(obj).depend target
include $(SRCTREE)/rules.mk

sinclude $(obj).depend

#########################################################################
