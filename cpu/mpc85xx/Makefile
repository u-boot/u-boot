#
# (C) Copyright 2006
# Wolfgang Denk, DENX Software Engineering, wd@denx.de.
#
# (C) Copyright 2002,2003 Motorola Inc.
# Xianghua Xiao,X.Xiao@motorola.com
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

LIB	= $(obj)lib$(CPU).a

START	= start.o resetvec.o
SOBJS-$(CONFIG_MP) += release.o
SOBJS	= $(SOBJS-y)
COBJS-$(CONFIG_MP) += mp.o
COBJS-$(CONFIG_OF_LIBFDT) += fdt.o
COBJS	= traps.o cpu.o cpu_init.o speed.o interrupts.o tlb.o \
	  pci.o serial_scc.o commproc.o ether_fcc.o spd_sdram.o qe_io.o \
	  $(COBJS-y)

SRCS	:= $(START:.o=.S) $(SOBJS:.o=.S) $(COBJS:.o=.c)
OBJS	:= $(addprefix $(obj),$(SOBJS) $(COBJS))
START	:= $(addprefix $(obj),$(START))

all:	$(obj).depend $(START) $(LIB)

$(LIB):	$(OBJS)
	$(AR) $(ARFLAGS) $@ $(OBJS)

#########################################################################

# defines $(obj).depend target
include $(SRCTREE)/rules.mk

sinclude $(obj).depend

#########################################################################
