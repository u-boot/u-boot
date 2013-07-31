#
# (C) Copyright 2000-2007
# Wolfgang Denk, DENX Software Engineering, wd@denx.de.
#
# SPDX-License-Identifier:	GPL-2.0+
#

include $(TOPDIR)/config.mk

LIB	:= $(obj)libusb_musb.o

COBJS-$(CONFIG_MUSB_HCD) += musb_hcd.o musb_core.o
COBJS-$(CONFIG_MUSB_UDC) += musb_udc.o musb_core.o
COBJS-$(CONFIG_USB_BLACKFIN) += blackfin_usb.o
COBJS-$(CONFIG_USB_DAVINCI) += davinci.o
COBJS-$(CONFIG_USB_OMAP3) += omap3.o
COBJS-$(CONFIG_USB_DA8XX) += da8xx.o
COBJS-$(CONFIG_USB_AM35X) += am35x.o

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
