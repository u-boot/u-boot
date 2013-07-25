#
# (C) Copyright 2000-2006
# Wolfgang Denk, DENX Software Engineering, wd@denx.de.
#
# SPDX-License-Identifier:	GPL-2.0+
#

include $(TOPDIR)/config.mk

# CFLAGS += -DET_DEBUG

LIB	= $(obj)lib$(CPU).o

SSTART-y += start.o
CSTART-y += traps.o
COBJS-y	+= bedbug_860.o
COBJS-y	+= commproc.o
COBJS-y	+= cpu.o
COBJS-y	+= cpu_init.o
COBJS-y	+= fec.o
COBJS-$(CONFIG_OF_LIBFDT) += fdt.o
COBJS-y	+= i2c.o
COBJS-y	+= interrupts.o
COBJS-y	+= scc.o
COBJS-y	+= serial.o
COBJS-y	+= speed.o
COBJS-y	+= spi.o
COBJS-y	+= upatch.o
COBJS-y	+= video.o
SOBJS-y	+= kgdb.o
SOBJS-y	+= plprcr_write.o

SRCS	:= $(SSTART-y:.o=.S) $(CSTART-y:.o=.c) $(SOBJS-y:.o=.S) $(COBJS-y:.o=.c)
OBJS	:= $(addprefix $(obj),$(SOBJS-y) $(COBJS-y))
START	:= $(addprefix $(obj),$(SSTART-y) $(CSTART-y))

all:	$(obj).depend $(START) $(LIB)

$(LIB):	$(OBJS)
	$(call cmd_link_o_target, $(OBJS))

#########################################################################

# defines $(obj).depend target
include $(SRCTREE)/rules.mk

sinclude $(obj).depend

#########################################################################
