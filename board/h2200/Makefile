#
# h2200 Support
#
# Copyright (C) 2012 Lukasz Dalek <luk0104@gmail.com>
#
# SPDX-License-Identifier:	GPL-2.0+
#

include $(TOPDIR)/config.mk

LIB	= $(obj)lib$(BOARD).o

COBJS	:= h2200.o

SRCS	:= $(COBJS:.o=.c) h2200-header.S
OBJS	:= $(addprefix $(obj),$(COBJS))

all: $(LIB) $(obj)h2200-header.bin

$(obj)h2200-header.o: h2200-header.S
	$(CC) $(CFLAGS) -c -o $@ $<

$(obj)h2200-header.bin: $(obj)h2200-header.o
	$(OBJCOPY) -O binary $< $@

$(LIB):	$(obj).depend $(OBJS)
	$(call cmd_link_o_target, $(OBJS))

#########################################################################

# defines $(obj).depend target
include $(SRCTREE)/rules.mk

sinclude $(obj).depend

#########################################################################
