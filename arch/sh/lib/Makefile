#
# Copyright (c) 2007
# Nobuhiro Iwamatsu <iwamatsu@nigauri.org>
#
# SPDX-License-Identifier:	GPL-2.0+
#

include $(TOPDIR)/config.mk

LIB	= $(obj)lib$(ARCH).o
LIBGCC	= $(obj)libgcc.o

SOBJS-y	+=
GLSOBJS	+= ashiftrt.o
GLSOBJS	+= ashiftlt.o
GLSOBJS	+= lshiftrt.o
GLSOBJS	+= ashldi3.o
GLSOBJS	+= ashrsi3.o
GLSOBJS	+= lshrdi3.o
GLSOBJS	+= movmem.o

COBJS-y	+= board.o
COBJS-$(CONFIG_CMD_BOOTM) += bootm.o
ifeq ($(CONFIG_SH2),y)
COBJS-y	+= time_sh2.o
else
COBJS-y	+= time.o
endif
ifeq ($(CONFIG_CMD_SH_ZIMAGEBOOT),y)
COBJS-y += zimageboot.o
endif

SRCS	:= $(SOBJS-y:.o=.S) $(COBJS-y:.o=.c)
OBJS	:= $(addprefix $(obj),$(SOBJS-y) $(COBJS-y))
LGOBJS	:= $(addprefix $(obj),$(GLSOBJS)) \
	   $(addprefix $(obj),$(GLCOBJS))

# Always build libsh.o
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
