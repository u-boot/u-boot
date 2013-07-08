#
# Copyright (C) 2012 Samsung Electronics
#
# SPDX-License-Identifier:	GPL-2.0+
#

include $(TOPDIR)/config.mk

LIB	= $(obj)lib$(BOARD).o

COBJS	+= smdk5250_spl.o

ifndef CONFIG_SPL_BUILD
ifdef CONFIG_OF_CONTROL
COBJS	+= exynos5-dt.o
else
COBJS	+= smdk5250.o
endif
endif

SRCS	:= $(COBJS:.o=.c)
OBJS	:= $(addprefix $(obj),$(COBJS))

ALL	:= $(obj).depend $(LIB)

all:	$(ALL)

$(LIB):	$(OBJS)
	$(call cmd_link_o_target, $(OBJS))

#########################################################################

# defines $(obj).depend target
include $(SRCTREE)/rules.mk

sinclude $(obj).depend

#########################################################################
