#
# Copyright (C) 2005-2006 Atmel Corporation
#
# SPDX-License-Identifier:	GPL-2.0+
#

include $(TOPDIR)/config.mk

LIB	:= $(obj)lib$(SOC).o

COBJS	:= portmux.o clk.o mmu.o
SRCS	:= $(SOBJS:.o=.S) $(COBJS:.o=.c)
OBJS	:= $(addprefix $(obj),$(SOBJS) $(COBJS))

all: $(obj).depend $(LIB)

$(LIB): $(OBJS)
	$(call cmd_link_o_target, $^)

#########################################################################

# defines $(obj).depend target
include $(SRCTREE)/rules.mk

sinclude $(obj).depend

#########################################################################
