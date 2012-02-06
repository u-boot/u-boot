#
# Copyright (C) 2011 Jana Rapava <fermata7@gmail.com>
# See file CREDITS for list of people who contributed to this
# project.
#
# This program is free software; you can redistribute it and/or
# modify it under the terms of the GNU General Public License as
# published by the Free Software Foundation; either version 2 of
# the License, or (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program; if not, write to the Free Software
# Foundation, Inc.
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
