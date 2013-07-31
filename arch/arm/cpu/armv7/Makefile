#
# (C) Copyright 2000-2003
# Wolfgang Denk, DENX Software Engineering, wd@denx.de.
#
# SPDX-License-Identifier:	GPL-2.0+
#

include $(TOPDIR)/config.mk

LIB	= $(obj)lib$(CPU).o

START	:= start.o

COBJS	+= cache_v7.o

COBJS	+= cpu.o
COBJS	+= syslib.o

ifneq ($(CONFIG_AM33XX)$(CONFIG_OMAP44XX)$(CONFIG_OMAP54XX)$(CONFIG_TEGRA)$(CONFIG_MX6)$(CONFIG_TI814X),)
SOBJS	+= lowlevel_init.o
endif

SRCS	:= $(START:.o=.S) $(COBJS:.o=.c)
OBJS	:= $(addprefix $(obj),$(COBJS) $(SOBJS))
START	:= $(addprefix $(obj),$(START))

all:	$(obj).depend $(START) $(LIB)

$(LIB):	$(OBJS)
	$(call cmd_link_o_target, $(OBJS))

#########################################################################

# defines $(obj).depend target
include $(SRCTREE)/rules.mk

sinclude $(obj).depend

#########################################################################
