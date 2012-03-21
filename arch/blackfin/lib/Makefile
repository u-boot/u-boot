#
# U-boot Makefile
#
# Copyright (c) 2005-2008 Analog Devices Inc.
#
# (C) Copyright 2000-2006
# Wolfgang Denk, DENX Software Engineering, wd@denx.de.
#
# See file CREDITS for list of people who contributed to this
# project.
#
# This program is free software; you can redistribute it and/or
# modify it under the terms of the GNU General Public License as
# published by the Free Software Foundation; either version 2 of
# the License, or (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program; if not, write to the Free Software
# Foundation, Inc., 51 Franklin St, Fifth Floor, Boston,
# MA 02110-1301 USA
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
COBJS-$(CONFIG_POST_ALT_LIST) += tests.o
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
