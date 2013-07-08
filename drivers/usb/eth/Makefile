#
# Copyright (c) 2011 The Chromium OS Authors.
# SPDX-License-Identifier:	GPL-2.0+
#

include $(TOPDIR)/config.mk

LIB	:= $(obj)libusb_eth.o

# new USB host ethernet layer dependencies
COBJS-$(CONFIG_USB_HOST_ETHER) += usb_ether.o
ifdef CONFIG_USB_ETHER_ASIX
COBJS-y += asix.o
endif
COBJS-$(CONFIG_USB_ETHER_SMSC95XX) += smsc95xx.o

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
