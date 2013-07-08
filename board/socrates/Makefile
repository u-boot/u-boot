#
# (C) Copyright 2008
# Sergei Poselenov,  Emcraft Systems, sposelenov@emcraft.com.
# (C) Copyright 2001-2006
# Wolfgang Denk, DENX Software Engineering, wd@denx.de.
#
# SPDX-License-Identifier:	GPL-2.0+
#

include $(TOPDIR)/config.mk

LIB	= $(obj)lib$(BOARD).o
#

COBJS-y	+= $(BOARD).o
COBJS-y	+= law.o
COBJS-y	+= tlb.o
COBJS-y	+= nand.o
COBJS-y	+= sdram.o
COBJS-$(CONFIG_FSL_DDR2) += ddr.o

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
