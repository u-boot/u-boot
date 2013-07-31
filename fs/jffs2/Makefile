#
# (C) Copyright 2000-2006
# Wolfgang Denk, DENX Software Engineering, wd@denx.de.
#
# SPDX-License-Identifier:	GPL-2.0+
#

include $(TOPDIR)/config.mk

LIB	= $(obj)libjffs2.o

AOBJS	=
ifdef CONFIG_CMD_JFFS2
COBJS-$(CONFIG_JFFS2_LZO) += compr_lzo.o
COBJS-y += compr_rtime.o
COBJS-y += compr_rubin.o
COBJS-y += compr_zlib.o
COBJS-y += jffs2_1pass.o
COBJS-y += mini_inflate.o
endif

COBJS	:= $(COBJS-y)
SRCS	:= $(AOBJS:.o=.S) $(COBJS:.o=.c)
OBJS	:= $(addprefix $(obj),$(AOBJS) $(COBJS))

#CPPFLAGS +=

all:	$(LIB) $(AOBJS)

$(LIB):	$(obj).depend $(OBJS)
	$(call cmd_link_o_target, $(OBJS))


#########################################################################

# defines $(obj).depend target
include $(SRCTREE)/rules.mk

sinclude $(obj).depend

#########################################################################
