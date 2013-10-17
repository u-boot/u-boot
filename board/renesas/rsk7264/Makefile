#
# Copyright (C) 2011 Renesas Electronics Europe Ltd.
#
# SPDX-License-Identifier:	GPL-2.0+

include $(TOPDIR)/config.mk

LIB	= lib$(BOARD).o

OBJS	:= rsk7264.o
SOBJS	:= lowlevel_init.o

LIB	:= $(addprefix $(obj),$(LIB))
OBJS	:= $(addprefix $(obj),$(OBJS))
SOBJS	:= $(addprefix $(obj),$(SOBJS))

$(LIB):	$(obj).depend $(OBJS) $(SOBJS)
	$(call cmd_link_o_target, $(OBJS) $(SOBJS))

#########################################################################

# defines $(obj).depend target
include $(SRCTREE)/rules.mk

sinclude $(obj).depend
