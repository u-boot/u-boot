#
# Copyright (c) 2009 Wind River Systems, Inc.
# Tom Rix <Tom.Rix@windriver.com>
#
# SPDX-License-Identifier:	GPL-2.0+
#

include $(TOPDIR)/config.mk

LIB	:= $(obj)libusb_phy.o

COBJS-$(CONFIG_TWL4030_USB) += twl4030.o
COBJS-$(CONFIG_OMAP_USB_PHY) += omap_usb_phy.o

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
