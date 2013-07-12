#
# Palm Treo680 Support
#
# Copyright (C) 2013 Mike Dunn <mikedunn@newsguy.com>
#
# This file is released under the terms of GPL v2 and any later version.
# See the file COPYING in the root directory of the source tree for details.

include $(TOPDIR)/config.mk

LIB	= $(obj)lib$(BOARD).o

COBJS	:= palmtreo680.o

SRCS	:= $(COBJS:.o=.c)
OBJS	:= $(addprefix $(obj),$(COBJS))

$(LIB):	$(obj).depend $(OBJS)
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
