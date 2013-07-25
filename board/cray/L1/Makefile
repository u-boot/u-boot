#
# (C) Copyright 2000-2006
# Wolfgang Denk, DENX Software Engineering, wd@denx.de.
#
# SPDX-License-Identifier:	GPL-2.0+
#

include $(TOPDIR)/config.mk

LIB	= $(obj)lib$(BOARD).o

COBJS	= $(BOARD).o flash.o
SOBJS	= init.o


SRCS	:= $(SOBJS:.o=.S) $(COBJS:.o=.c)
OBJS	:= $(addprefix $(obj),$(COBJS))
SOBJS	:= $(addprefix $(obj),$(SOBJS))

# HACK: depend needs bootscript.c, which needs tools/mkimage, which is not
# built in the depend stage.  So... put bootscript.o here, not in OBJS
$(LIB):	$(OBJS) $(SOBJS) $(obj)bootscript.o
	$(call cmd_link_o_target, $^)

$(obj)$(BOARD).o : $(src)$(BOARD).c $(obj)bootscript.o

$(obj)bootscript.c: $(obj)bootscript.image
	od -t x1 -v -A x $^ | awk -f x2c.awk > $@

$(obj)bootscript.image: $(src)bootscript.hush $(src)Makefile
	-$(OBJTREE)/tools/mkimage -A ppc -O linux -T script -C none -a 0 -e 0 -n bootscript -d $(src)bootscript.hush $@

#########################################################################

# defines $(obj).depend target
include $(SRCTREE)/rules.mk

sinclude $(obj).depend

#########################################################################
