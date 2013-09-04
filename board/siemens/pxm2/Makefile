#
# Makefile
#
# (C) Copyright 2013 Siemens Schweiz AG
# (C) Heiko Schocher, DENX Software Engineering, hs@denx.de.
#
# Based on:
# u-boot:/board/ti/am335x/Makefile
# Copyright (C) 2011 Texas Instruments Incorporated - http://www.ti.com/
#
# SPDX-License-Identifier:	GPL-2.0+
#

include $(TOPDIR)/config.mk
ifneq ($(OBJTREE),$(SRCTREE))
$(shell mkdir -p $(obj)../common)
endif

LIB	= $(obj)lib$(BOARD).o

ifdef CONFIG_SPL_BUILD
COBJS	:= mux.o
endif

COBJS	+= board.o
ifndef CONFIG_SPL_BUILD
COBJS += ../common/factoryset.o
endif
SRCS	:= $(SOBJS:.o=.S) $(COBJS:.o=.c)
OBJS	:= $(addprefix $(obj),$(COBJS))
SOBJS	:= $(addprefix $(obj),$(SOBJS))

$(LIB):	$(obj).depend $(OBJS) $(SOBJS)
	$(call cmd_link_o_target, $(OBJS) $(SOBJS))

clean:
	rm -f $(SOBJS) $(OBJS)

distclean:	clean
	rm -f $(LIB) core *.bak $(obj).depend

#########################################################################

# defines $(obj).depend target
include $(SRCTREE)/rules.mk

sinclude $(obj).depend

#########################################################################
