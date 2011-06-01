#
# Copyright (C) 2011 Renesas Electronics Europe Ltd.
#
# This file is released under the terms of GPL v2 and any later version.
# See the file COPYING in the root directory of the source tree for details.

include $(TOPDIR)/config.mk

LIB	= lib$(BOARD).o

OBJS	:= rsk7264.o
SOBJS	:= lowlevel_init.o

LIB	:= $(addprefix $(obj),$(LIB))
OBJS	:= $(addprefix $(obj),$(OBJS))
SOBJS	:= $(addprefix $(obj),$(SOBJS))

$(LIB):	$(obj).depend $(OBJS) $(SOBJS)
	$(call cmd_link_o_target, $(OBJS) $(SOBJS))

clean:
	rm -f $(SOBJS) $(OBJS)

distclean:	clean
	rm -f $(LIB) core *.bak .depend

#########################################################################

# defines $(obj).depend target
include $(SRCTREE)/rules.mk

sinclude $(obj).depend
