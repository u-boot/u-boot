#
# Copyright (c) 2000 - 2013 Samsung Electronics Co., Ltd. All rights reserved.
# Sanghee Kim <sh0130.kim@samsung.com>
#
# SPDX-License-Identifier:	GPL-2.0+
#

include $(TOPDIR)/config.mk

LIB	= $(obj)lib$(BOARD).o

COBJS-y	:= trats2.o

SRCS    := $(SOBJS:.o=.S) $(COBJS-y:.o=.c)
OBJS	:= $(addprefix $(obj),$(COBJS-y))


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
