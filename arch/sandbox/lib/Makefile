#
# Copyright (c) 2011 The Chromium OS Authors.
#
# (C) Copyright 2002-2006
# Wolfgang Denk, DENX Software Engineering, wd@denx.de.
#
# SPDX-License-Identifier:	GPL-2.0+
#

include $(TOPDIR)/config.mk

LIB	= $(obj)lib$(ARCH).o

COBJS-y	+= interrupts.o

SRCS	:= $(COBJS-y:.o=.c)
OBJS	:= $(addprefix $(obj),$(COBJS-y))

# Always build libsandbox.o
TARGETS	:= $(LIB)

all:	$(TARGETS)

$(LIB):	$(obj).depend $(OBJS)
	$(call cmd_link_o_target, $(OBJS))

#########################################################################

# defines $(obj).depend target
include $(SRCTREE)/rules.mk

sinclude $(obj).depend

#########################################################################
