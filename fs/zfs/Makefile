#
# (C) Copyright 2012
# Jorgen Lundman <lundman at lundman.net>
#
# SPDX-License-Identifier:	GPL-2.0+
#

include $(TOPDIR)/config.mk

LIB	= $(obj)libzfs.o

AOBJS	=
COBJS-$(CONFIG_CMD_ZFS) := dev.o zfs.o zfs_fletcher.o zfs_sha256.o zfs_lzjb.o

SRCS	:= $(AOBJS:.o=.S) $(COBJS-y:.o=.c)
OBJS	:= $(addprefix $(obj),$(AOBJS) $(COBJS-y))


all:	$(LIB) $(AOBJS)

$(LIB):	$(obj).depend $(OBJS)
	$(call cmd_link_o_target, $(OBJS))

#########################################################################

# defines $(obj).depend target
include $(SRCTREE)/rules.mk

sinclude $(obj).depend

#########################################################################
