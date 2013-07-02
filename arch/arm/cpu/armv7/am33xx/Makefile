#
# Copyright (C) 2011, Texas Instruments, Incorporated - http://www.ti.com/
#
# SPDX-License-Identifier:	GPL-2.0+
#

include $(TOPDIR)/config.mk

LIB	= $(obj)lib$(SOC).o

COBJS-$(CONFIG_AM33XX)	+= clock_am33xx.o
COBJS-$(CONFIG_TI814X)	+= clock_ti814x.o
COBJS-$(CONFIG_AM43XX)	+= clock_am43xx.o

ifneq ($(CONFIG_AM43XX)$(CONFIG_AM33XX),)
COBJS	+= clock.o
endif

COBJS-$(CONFIG_TI816X)	+= clock_ti816x.o
COBJS	+= sys_info.o
COBJS	+= mem.o
COBJS	+= ddr.o
COBJS	+= emif4.o
COBJS	+= board.o
COBJS	+= mux.o
COBJS-$(CONFIG_NAND_OMAP_GPMC)	+= elm.o

SRCS	:= $(SOBJS:.o=.S) $(COBJS:.o=.c)
OBJS	:= $(addprefix $(obj),$(COBJS) $(COBJS-y) $(SOBJS))

all:	$(obj).depend $(LIB)

$(LIB):	$(OBJS)
	$(call cmd_link_o_target, $(OBJS))

clean:
	rm -f $(SOBJS) $(OBJS)

distclean:	clean
	rm -f $(LIB) core *.bak .depend

#########################################################################

# defines $(obj).depend target
include $(SRCTREE)/rules.mk

sinclude $(obj).depend

#########################################################################
