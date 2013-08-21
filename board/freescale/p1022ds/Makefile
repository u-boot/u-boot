#
# Copyright 2010 Freescale Semiconductor, Inc.
#
# SPDX-License-Identifier:	GPL-2.0+
#

include $(TOPDIR)/config.mk

LIB	= $(obj)lib$(BOARD).o

MINIMAL=

ifdef CONFIG_SPL_BUILD
ifdef CONFIG_SPL_INIT_MINIMAL
MINIMAL=y
endif
endif

ifdef MINIMAL

COBJS-y        += spl_minimal.o tlb.o law.o

else
ifdef CONFIG_SPL_BUILD
COBJS-y += spl.o
endif
COBJS-y	+= $(BOARD).o
COBJS-y	+= ddr.o
COBJS-y	+= law.o
COBJS-y	+= tlb.o

COBJS-$(CONFIG_FSL_DIU_FB) += diu.o
endif

SRCS	:= $(SOBJS:.o=.S) $(COBJS-y:.o=.c)
OBJS	:= $(addprefix $(obj),$(COBJS-y))
SOBJS	:= $(addprefix $(obj),$(SOBJS))

$(LIB):	$(obj).depend $(OBJS) $(SOBJS)
	$(call cmd_link_o_target, $(OBJS))

#########################################################################

# defines $(obj).depend target
include $(SRCTREE)/rules.mk

sinclude $(obj).depend

#########################################################################
