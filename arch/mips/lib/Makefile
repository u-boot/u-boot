#
# (C) Copyright 2003-2006
# Wolfgang Denk, DENX Software Engineering, wd@denx.de.
#
# SPDX-License-Identifier:	GPL-2.0+
#

include $(TOPDIR)/config.mk

LIB	= $(obj)lib$(ARCH).o

## Build a couple of necessary functions into a private libgcc
LIBGCC	= $(obj)libgcc.o
GLSOBJS	+= ashldi3.o
GLSOBJS	+= ashrdi3.o
GLSOBJS	+= lshrdi3.o
LGOBJS	:= $(addprefix $(obj),$(GLSOBJS))

SOBJS-y	+=

COBJS-y	+= board.o
COBJS-$(CONFIG_CMD_BOOTM) += bootm.o

SRCS	:= $(SOBJS-y:.o=.S) $(COBJS-y:.o=.c)
OBJS	:= $(addprefix $(obj),$(SOBJS-y) $(COBJS-y))

# Always build libmips.o
TARGETS	:= $(LIB)

# Build private libgcc only when asked for
ifdef USE_PRIVATE_LIBGCC
TARGETS	+= $(LIBGCC)
endif

all:	$(TARGETS)

$(LIB):	$(obj).depend $(OBJS)
	$(call cmd_link_o_target, $(OBJS))

$(LIBGCC): $(obj).depend $(LGOBJS)
	$(call cmd_link_o_target, $(LGOBJS))

#########################################################################

# defines $(obj).depend target
include $(SRCTREE)/rules.mk

sinclude $(obj).depend

#########################################################################
