#
# (C) Copyright 2006
# Wolfgang Denk, DENX Software Engineering, wd@denx.de.
#
# (C) Copyright 2005
# Travis B. Sawyer, Sandburst Corporation, tsawyer@sandburst.com
#
# SPDX-License-Identifier:	GPL-2.0+
#

include $(TOPDIR)/config.mk
ifneq ($(OBJTREE),$(SRCTREE))
$(shell mkdir -p $(obj)../common)
endif

# TBS: add for debugging purposes
BUILDUSER := $(shell whoami)
FORCEBUILD := $(shell rm -f $(LIB) $(BOARD).o)

CFLAGS += -DBUILDUSER='"$(BUILDUSER)"'
# TBS: end debugging


LIB	= $(obj)lib$(BOARD).o

COBJS	= $(BOARD).o ../common/flash.o ../common/sb_common.o
SOBJS	= init.o

SRCS	:= $(SOBJS:.o=.S) $(COBJS:.o=.c)
OBJS	:= $(addprefix $(obj),$(COBJS))
SOBJS	:= $(addprefix $(obj),$(SOBJS))

$(LIB):	$(OBJS) $(SOBJS)
	$(call cmd_link_o_target, $(OBJS))

#########################################################################

# defines $(obj).depend target
include $(SRCTREE)/rules.mk

sinclude $(obj).depend

#########################################################################
