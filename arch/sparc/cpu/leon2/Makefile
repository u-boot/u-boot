#
# (C) Copyright 2003-2006
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

LIB	= $(obj)lib$(CPU).o

START	= start.o
SOBJS	=
COBJS	= cpu_init.o serial.o cpu.o interrupts.o prom.o

SRCS	:= $(START:.o=.S) $(SOBJS:.o=.S) $(COBJS:.o=.c)
OBJS	:= $(addprefix $(obj),$(SOBJS) $(COBJS))
START	:= $(addprefix $(obj),$(START))

all:	$(obj).depend $(START) $(LIB)

$(LIB):	$(OBJS)
	$(call cmd_link_o_target, $(OBJS))

#########################################################################

# defines $(obj).depend target
include $(SRCTREE)/rules.mk

$(START): $(START:.o=.S)
	$(CC) -D__ASSEMBLY__ $(DBGFLAGS) $(OPTFLAGS) -D__KERNEL__ \
	-DCONFIG_SYS_TEXT_BASE=$(CONFIG_SYS_TEXT_BASE) -I$(TOPDIR)/include \
	-fno-builtin -ffreestanding -nostdinc -isystem $(gccincdir) -pipe \
	$(PLATFORM_CPPFLAGS) -Wall -Wstrict-prototypes \
	-I$(TOPDIR)/board -c -o $(START) $(START:.o=.S)

sinclude $(obj).depend

#########################################################################
