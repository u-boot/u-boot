#
# (C) Copyright 2006
# Wolfgang Denk, DENX Software Engineering, wd@denx.de.
#
# (C) Copyright 2003
# Pavel Bartusek, Sysgo Real-Time Solutions AG, pba@sysgo.de
#
#
# SPDX-License-Identifier:	GPL-2.0+
#

include $(TOPDIR)/config.mk

LIB	= $(obj)libubifs.o

COBJS-$(CONFIG_CMD_UBIFS) := ubifs.o io.o super.o sb.o master.o lpt.o
COBJS-$(CONFIG_CMD_UBIFS) += lpt_commit.o scan.o lprops.o
COBJS-$(CONFIG_CMD_UBIFS) += tnc.o tnc_misc.o debug.o crc16.o budget.o
COBJS-$(CONFIG_CMD_UBIFS) += log.o orphan.o recovery.o replay.o

SRCS	:= $(AOBJS:.o=.S) $(COBJS-y:.o=.c)
OBJS	:= $(addprefix $(obj),$(AOBJS) $(COBJS-y))

all:	$(LIB) $(AOBJS)

$(LIB):	$(obj).depend $(OBJS)
	$(call cmd_link_o_target, $(OBJS))

# SEE README.arm-unaligned-accesses
$(obj)super.o: CFLAGS += $(PLATFORM_NO_UNALIGNED)

#########################################################################

# defines $(obj).depend target
include $(SRCTREE)/rules.mk

sinclude $(obj).depend

#########################################################################
