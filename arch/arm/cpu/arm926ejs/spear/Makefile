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
# Foundation, Inc., 59 Temple Place, Suite 330, Boston,
# MA 02111-1307 USA
#

include $(TOPDIR)/config.mk

LIB	= $(obj)lib$(SOC).o

COBJS-y	:= cpu.o \
	   reset.o \
	   timer.o

ifdef CONFIG_SPL_BUILD
COBJS-y	+= spl.o spl_boot.o
COBJS-$(CONFIG_SPEAR600) += spear600.o
COBJS-$(CONFIG_DDR_MT47H64M16) += spr600_mt47h64m16_3_333_cl5_psync.o
COBJS-$(CONFIG_DDR_MT47H32M16) += spr600_mt47h32m16_333_cl5_psync.o
COBJS-$(CONFIG_DDR_MT47H32M16) += spr600_mt47h32m16_37e_166_cl4_sync.o
COBJS-$(CONFIG_DDR_MT47H128M8) += spr600_mt47h128m8_3_266_cl5_async.o
endif

SRCS	:= $(START:.o=.S) $(COBJS-y:.o=.c)
OBJS	:= $(addprefix $(obj),$(COBJS-y))
START	:= $(addprefix $(obj),$(START))

all:	$(obj).depend $(LIB)

$(LIB):	$(OBJS)
	$(call cmd_link_o_target, $(OBJS))

#########################################################################

# defines $(obj).depend target
include $(SRCTREE)/rules.mk

sinclude $(obj).depend

#########################################################################
