#
# Copyright (C) 2011 Samsung Electronics
#
# SPDX-License-Identifier:	GPL-2.0+
#

include $(TOPDIR)/config.mk

LIB	= $(obj)lib$(BOARD).o

ifndef CONFIG_SPL_BUILD
COBJS	+= origen.o
endif

SRCS	:= $(COBJS:.o=.c)
OBJS	:= $(addprefix $(obj),$(COBJS))

ALL	+=$(obj).depend $(LIB)

ifdef CONFIG_SPL_BUILD
ALL	+= $(OBJTREE)/tools/mk$(BOARD)spl
endif

all:	$(ALL)

$(LIB):	$(OBJS)
	$(call cmd_link_o_target, $(OBJS))

ifdef CONFIG_SPL_BUILD
$(OBJTREE)/tools/mk$(BOARD)spl:	tools/mkv310_image.c
	$(HOSTCC) tools/mkv310_image.c -o $(OBJTREE)/tools/mk$(BOARD)spl
endif

#########################################################################

# defines $(obj).depend target
include $(SRCTREE)/rules.mk

sinclude $(obj).depend

#########################################################################
