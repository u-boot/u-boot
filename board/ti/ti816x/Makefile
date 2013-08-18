#
# Copyright (C) 2013, Adeneo Embedded <www.adeneo-embedded.com>
# Antoine Tenart, <atenart@adeneo-embedded.com>
#
# Based on TI-PSP-04.00.02.14 :
#
# Copyright (C) 2009, Texas Instruments, Incorporated
#
# SPDX-License-Identifier:	GPL-2.0
#

include $(TOPDIR)/config.mk

LIB	= $(obj)lib$(BOARD).o

COBJS	:= evm.o

SRCS	:= $(SOBJS:.o=.S) $(COBJS:.o=.c)
OBJS	:= $(addprefix $(obj),$(COBJS))

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
