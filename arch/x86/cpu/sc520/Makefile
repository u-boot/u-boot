#
# (C) Copyright 2008
# Graeme Russ, graeme.russ@gmail.com.
#
# (C) Copyright 2006
# Wolfgang Denk, DENX Software Engineering, wd@denx.de.
#
# (C) Copyright 2002
# Daniel Engström, Omicron Ceti AB, daniel@omicron.se.
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

LIB	:= $(obj)lib$(SOC).o

COBJS-$(CONFIG_SYS_SC520) += sc520.o
COBJS-$(CONFIG_PCI) += sc520_pci.o
COBJS-$(CONFIG_SYS_SC520_RESET) += sc520_reset.o
COBJS-$(CONFIG_SYS_SC520) += sc520_sdram.o
COBJS-$(CONFIG_SYS_SC520_SSI) += sc520_ssi.o
COBJS-$(CONFIG_SYS_SC520_TIMER) += sc520_timer.o

SOBJS-$(CONFIG_SYS_SC520) += sc520_car.o

SRCS	:= $(SOBJS-y:.o=.S) $(COBJS-y:.o=.c)
OBJS	:= $(addprefix $(obj),$(SOBJS-y) $(COBJS-y))

all: $(obj).depend $(LIB)

$(LIB):	$(OBJS)
	$(call cmd_link_o_target, $(OBJS))

#########################################################################

# defines $(obj).depend target
include $(SRCTREE)/rules.mk

sinclude $(obj).depend

#########################################################################