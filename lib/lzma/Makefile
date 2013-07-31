#
# Copyright (C) 2007-2008 Industrie Dial Face S.p.A.
# Luigi 'Comio' Mantellini (luigi.mantellini@idf-hit.com)
#
# (C) Copyright 2003-2006
# Wolfgang Denk, DENX Software Engineering, wd@denx.de.
#
# SPDX-License-Identifier:	GPL-2.0+
#

include $(TOPDIR)/config.mk

LIB	= $(obj)liblzma.o

SOBJS	=

CFLAGS += -D_LZMA_PROB32

COBJS-$(CONFIG_LZMA) += LzmaDec.o LzmaTools.o

COBJS	= $(COBJS-y)
SRCS 	:= $(SOBJS:.o=.S) $(COBJS:.o=.c)
OBJS	:= $(addprefix $(obj),$(SOBJS) $(COBJS))

$(LIB):	$(obj).depend $(OBJS)
	$(call cmd_link_o_target, $(OBJS))

#########################################################################

# defines $(obj).depend target
include $(SRCTREE)/rules.mk

sinclude $(obj).depend

#########################################################################
