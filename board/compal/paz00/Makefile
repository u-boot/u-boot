#
# Copyright (c) 2010-2012, NVIDIA CORPORATION.  All rights reserved.
#
# See file CREDITS for list of people who contributed to this
# project.
#
# This program is free software; you can redistribute it and/or modify it
# under the terms and conditions of the GNU General Public License,
# version 2, as published by the Free Software Foundation.
#
# This program is distributed in the hope it will be useful, but WITHOUT
# ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
# FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for
# more details.
#

include $(TOPDIR)/config.mk

ifneq ($(OBJTREE),$(SRCTREE))
$(shell mkdir -p $(obj)../../nvidia/common)
endif

LIB	= $(obj)lib$(BOARD).o

COBJS	:= $(BOARD).o
COBJS	+= ../../nvidia/common/board.o

SRCS	:= $(COBJS:.o=.c)
OBJS	:= $(addprefix $(obj),$(COBJS))

$(LIB):	$(obj).depend $(OBJS)
	$(call cmd_link_o_target, $(OBJS))

#########################################################################

# defines $(obj).depend target
include $(SRCTREE)/rules.mk

sinclude $(obj).depend

#########################################################################
