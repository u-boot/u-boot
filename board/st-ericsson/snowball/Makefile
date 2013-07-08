#
# Copyright (C) ST-Ericsson SA 2009
#
# SPDX-License-Identifier:	GPL-2.0+
#

include $(TOPDIR)/config.mk

CFLAGS += -D__RELEASE -D__STN_8500
LIB	= $(obj)lib$(BOARD).o

COBJS	:= snowball.o

SRCS	:= $(COBJS:.o=.c)
OBJS	:= $(addprefix $(obj),$(COBJS))

$(LIB): $(obj).depend $(OBJS)
	$(call cmd_link_o_target, $(OBJS))

clean:
	rm -f $(OBJS)

distclean:	clean
	rm -f $(LIB) core *.bak $(obj).depend

#########################################################################

# defines $(obj).depend target
include $(SRCTREE)/rules.mk

sinclude $(obj).depend

#########################################################################
