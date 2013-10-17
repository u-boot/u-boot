#
# (C) Copyright 2000-2006
# Wolfgang Denk, DENX Software Engineering, wd@denx.de.
#
# SPDX-License-Identifier:	GPL-2.0+
#

include $(TOPDIR)/config.mk

LIB	= $(obj)lib$(SOC).o

SOBJS = lowlevel_init.o
COBJS-y += cpu_info.o
COBJS-y += emac.o

COBJS-$(CONFIG_DISPLAY_BOARDINFO) += board.o
COBJS-$(CONFIG_GLOBAL_TIMER) += timer.o
COBJS-$(CONFIG_R8A7740) += cpu_info-r8a7740.o
COBJS-$(CONFIG_R8A7740) += pfc-r8a7740.o
COBJS-$(CONFIG_SH73A0) += cpu_info-sh73a0.o
COBJS-$(CONFIG_SH73A0) += pfc-sh73a0.o
COBJS_LN-$(CONFIG_TMU_TIMER) += sh_timer.o

COBJS	:= $(COBJS-y)
SRCS    := $(SOBJS:.o=.S) $(COBJS:.o=.c)
SRCS    := $(SOBJS:.o=.S) $(COBJS:.o=.c) $(addprefix $(obj),$(COBJS_LN-y:.o=.c))
OBJS	:= $(addprefix $(obj),$(SOBJS) $(COBJS) $(COBJS_LN-y))
SOBJS	:= $(addprefix $(obj),$(SOBJS))
START	:= $(addprefix $(obj),$(START))

all:	$(obj).depend $(LIB)

$(LIB):	$(OBJS)
	$(call cmd_link_o_target, $(OBJS))

# from arch/sh/lib/ directory
$(obj)sh_timer.c:
	@rm -f $(obj)sh_timer.c
	ln -s $(SRCTREE)/arch/sh/lib/time.c $(obj)sh_timer.c

#########################################################################

# defines $(obj).depend target
include $(SRCTREE)/rules.mk

sinclude $(obj).depend

#########################################################################
