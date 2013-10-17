#
# U-boot Makefile
#
# Copyright (c) 2005-2008 Analog Devices Inc.
#
# (C) Copyright 2000-2006
# Wolfgang Denk, DENX Software Engineering, wd@denx.de.
#
# SPDX-License-Identifier:	GPL-2.0+
#

include $(TOPDIR)/config.mk

CFLAGS += -DBFIN_BOARD_NAME='"$(BOARD)"'

LIB	= $(obj)lib$(ARCH).o

SOBJS-y	+= ins.o
SOBJS-y	+= memcmp.o
SOBJS-y	+= memcpy.o
SOBJS-y	+= memmove.o
SOBJS-y	+= memset.o
SOBJS-y	+= outs.o
SOBJS-$(CONFIG_CMD_KGDB) += __kgdb.o

COBJS-y	+= board.o
COBJS-y	+= boot.o
COBJS-y	+= cache.o
COBJS-y	+= clocks.o
COBJS-$(CONFIG_CMD_CACHE_DUMP) += cmd_cache_dump.o
COBJS-$(CONFIG_CMD_KGDB) += kgdb.o
COBJS-y	+= muldi3.o
COBJS-$(CONFIG_HAS_POST) += post.o
COBJS-y	+= string.o

SRCS	:= $(SOBJS-y:.o=.S) $(COBJS-y:.o=.c)
OBJS	:= $(addprefix $(obj),$(SOBJS-y) $(COBJS-y))

$(LIB):	$(obj).depend $(OBJS)
	$(call cmd_link_o_target, $(OBJS))

#########################################################################

# defines $(obj).depend target
include $(SRCTREE)/rules.mk

sinclude $(obj).depend

#########################################################################
