#
# (C) Copyright 2000-2006
# Wolfgang Denk, DENX Software Engineering, wd@denx.de.
#
# SPDX-License-Identifier:	GPL-2.0+
#

include $(TOPDIR)/config.mk
ifneq ($(OBJTREE),$(SRCTREE))
$(shell mkdir -p $(obj)drivers/epic $(obj)drivers/i2c)
endif

LIB	= $(obj)lib$(CPU).o

START	= start.o
COBJS	= traps.o cpu.o cpu_init.o interrupts.o speed.o \
	  drivers/epic/epic1.o drivers/i2c/i2c.o pci.o
COBJS_LN = bedbug_603e.o

SRCS	:= $(START:.o=.S) $(SOBJS:.o=.S) $(COBJS:.o=.c) $(addprefix $(obj),$(COBJS_LN:.o=.c))
OBJS	:= $(addprefix $(obj),$(SOBJS) $(COBJS) $(COBJS_LN))
START	:= $(addprefix $(obj),$(START))

all:	$(obj).depend $(START) $(LIB)

$(LIB):	$(OBJS)
	$(call cmd_link_o_target, $(OBJS))

$(obj)bedbug_603e.c:
	ln -sf $(src)../mpc8260/bedbug_603e.c $(obj)bedbug_603e.c

#########################################################################

# defines $(obj).depend target
include $(SRCTREE)/rules.mk

sinclude $(obj).depend

#########################################################################
