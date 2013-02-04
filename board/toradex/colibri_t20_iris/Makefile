#
#  (C) Copyright 2012 Lucas Stach
#
#  This program is free software; you can redistribute it and/or
#  modify it under the terms of the GNU General Public License as
#  published by the Free Software Foundation; either version 2 of
#  the License, or (at your option) any later version.
#
#  This program is distributed in the hope that it will be useful,
#  but WITHOUT ANY WARRANTY; without even the implied warranty of
#  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#  GNU General Public License for more details.
#
#

include $(TOPDIR)/config.mk

$(shell mkdir -p $(obj)../../nvidia/common)
$(shell mkdir -p $(obj)../colibri_t20-common)

LIB	= $(obj)lib$(BOARD).o

COBJS	:= ../../nvidia/common/board.o
COBJS	+= ../colibri_t20-common/colibri_t20-common.o
COBJS	+= $(BOARD).o

SRCS	:= $(COBJS:.o=.c)
OBJS	:= $(addprefix $(obj),$(COBJS))

$(LIB):	$(obj).depend $(OBJS)
	$(call cmd_link_o_target, $(OBJS))

#########################################################################

# defines $(obj).depend target
include $(SRCTREE)/rules.mk

sinclude $(obj).depend

#########################################################################
