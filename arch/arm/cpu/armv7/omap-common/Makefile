#
# (C) Copyright 2000-2003
# Wolfgang Denk, DENX Software Engineering, wd@denx.de.
#
# SPDX-License-Identifier:	GPL-2.0+
#

include $(TOPDIR)/config.mk

LIB	= $(obj)libomap-common.o

COBJS	:= reset.o
COBJS	+= timer.o
COBJS	+= utils.o

ifneq ($(CONFIG_OMAP44XX)$(CONFIG_OMAP54XX),)
COBJS	+= hwinit-common.o
COBJS	+= clocks-common.o
COBJS	+= emif-common.o
COBJS	+= vc.o
COBJS	+= abb.o
endif

ifeq ($(CONFIG_OMAP34XX),)
COBJS	+= boot-common.o
SOBJS	+= lowlevel_init.o
endif

ifndef CONFIG_SPL_BUILD
ifneq ($(CONFIG_OMAP44XX)$(CONFIG_OMAP54XX),)
COBJS	+= mem-common.o
endif
endif

SRCS	:= $(SOBJS:.o=.S) $(COBJS:.o=.c)
OBJS	:= $(addprefix $(obj),$(SOBJS) $(COBJS))

all:	$(obj).depend $(LIB)

$(LIB):	$(OBJS)
	$(call cmd_link_o_target, $(OBJS))

#########################################################################

# defines $(obj).depend target
include $(SRCTREE)/rules.mk

sinclude $(obj).depend

#########################################################################
