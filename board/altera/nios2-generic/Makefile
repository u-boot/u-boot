#
# (C) Copyright 2001-2006
# Wolfgang Denk, DENX Software Engineering, wd@denx.de.
# (C) Copyright 2010, Thomas Chou <thomas@wytron.com.tw>
#
# SPDX-License-Identifier:	GPL-2.0+
#

include $(TOPDIR)/config.mk
ifneq ($(OBJTREE),$(SRCTREE))
$(shell mkdir -p $(obj)../common)
endif

LIB	= $(obj)lib$(BOARD).o

COBJS-y	:= $(BOARD).o
COBJS-$(CONFIG_CMD_IDE) += ../common/cfide.o
COBJS-$(CONFIG_EPLED) += ../common/epled.o
COBJS-$(CONFIG_SEVENSEG) += ../common/sevenseg.o

SOBJS-y	:= text_base.o

SRCS	:= $(SOBJS-y:.o=.S) $(COBJS-y:.o=.c)
OBJS	:= $(addprefix $(obj),$(COBJS-y))
SOBJS	:= $(addprefix $(obj),$(SOBJS-y))

$(LIB):	$(obj).depend $(OBJS) $(SOBJS)
	$(call cmd_link_o_target, $(OBJS) $(SOBJS))

#########################################################################

# defines $(obj).depend target
include $(SRCTREE)/rules.mk

sinclude $(obj).depend

#########################################################################
