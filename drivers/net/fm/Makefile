#
# Copyright 2009-2011 Freescale Semiconductor, Inc.
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
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program; if not, write to the Free Software
# Foundation, Inc., 59 Temple Place, Suite 330, Boston,
# MA 02111-1307 USA
#

include $(TOPDIR)/config.mk

LIB	:= $(obj)libfm.o

ifdef CONFIG_FMAN_ENET
COBJS-y += dtsec.o
COBJS-y += eth.o
COBJS-y += fm.o
COBJS-y += init.o
COBJS-y += tgec.o
COBJS-y += tgec_phy.o

# SoC specific SERDES support
COBJS-$(CONFIG_P1017)	+= p1023.o
COBJS-$(CONFIG_P1023)	+= p1023.o
# The P204x, P304x, and P5020 are the same
COBJS-$(CONFIG_PPC_P2040) += p5020.o
COBJS-$(CONFIG_PPC_P2041) += p5020.o
COBJS-$(CONFIG_PPC_P3041) += p5020.o
COBJS-$(CONFIG_PPC_P3060) += p3060.o
COBJS-$(CONFIG_PPC_P4080) += p4080.o
COBJS-$(CONFIG_PPC_P5020) += p5020.o
endif

COBJS	:= $(COBJS-y)
SRCS	:= $(COBJS:.o=.c)
OBJS	:= $(addprefix $(obj),$(COBJS))

all:	$(LIB)

$(LIB):	$(obj).depend $(OBJS)
	$(call cmd_link_o_target, $(OBJS))

#########################################################################

include $(SRCTREE)/rules.mk

sinclude $(obj).depend

#########################################################################
