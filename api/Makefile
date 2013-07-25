#
# (C) Copyright 2007 Semihalf
#
# SPDX-License-Identifier:	GPL-2.0+
#

include $(TOPDIR)/config.mk

LIB	= $(obj)libapi.o

COBJS-$(CONFIG_API) += api.o api_display.o api_net.o api_storage.o \
		       api_platform-$(ARCH).o

COBJS	:= $(COBJS-y)
SRCS	:= $(COBJS:.o=.c)
OBJS	:= $(addprefix $(obj),$(COBJS))

$(LIB):	$(obj).depend $(OBJS)
	$(call cmd_link_o_target, $(OBJS))

# defines $(obj).depend target
include $(SRCTREE)/rules.mk

sinclude $(obj).depend
