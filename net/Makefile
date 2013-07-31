#
# (C) Copyright 2000-2006
# Wolfgang Denk, DENX Software Engineering, wd@denx.de.
#
# SPDX-License-Identifier:	GPL-2.0+
#

include $(TOPDIR)/config.mk

# CFLAGS += -DDEBUG

LIB	= $(obj)libnet.o

COBJS-$(CONFIG_CMD_NET)  += arp.o
COBJS-$(CONFIG_CMD_NET)  += bootp.o
COBJS-$(CONFIG_CMD_CDP)  += cdp.o
COBJS-$(CONFIG_CMD_DNS)  += dns.o
COBJS-$(CONFIG_CMD_NET)  += eth.o
COBJS-$(CONFIG_CMD_LINK_LOCAL) += link_local.o
COBJS-$(CONFIG_CMD_NET)  += net.o
COBJS-$(CONFIG_CMD_NFS)  += nfs.o
COBJS-$(CONFIG_CMD_PING) += ping.o
COBJS-$(CONFIG_CMD_RARP) += rarp.o
COBJS-$(CONFIG_CMD_SNTP) += sntp.o
COBJS-$(CONFIG_CMD_NET)  += tftp.o

COBJS	:= $(sort $(COBJS-y))
SRCS	:= $(COBJS:.o=.c)
OBJS	:= $(addprefix $(obj),$(COBJS))

all:	$(LIB)

$(LIB):	$(obj).depend $(OBJS)
	$(call cmd_link_o_target, $(OBJS))

#########################################################################

# defines $(obj).depend target
include $(SRCTREE)/rules.mk

sinclude $(obj).depend

#########################################################################
