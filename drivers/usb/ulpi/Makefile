#
# Copyright (C) 2011 Jana Rapava <fermata7@gmail.com>
#
# SPDX-License-Identifier:	GPL-2.0+
#

include $(TOPDIR)/config.mk

LIB	:= $(obj)libusb_ulpi.o

COBJS-$(CONFIG_USB_ULPI)		+= ulpi.o
COBJS-$(CONFIG_USB_ULPI_VIEWPORT)	+= ulpi-viewport.o
COBJS-$(CONFIG_USB_ULPI_VIEWPORT_OMAP)	+= omap-ulpi-viewport.o

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
