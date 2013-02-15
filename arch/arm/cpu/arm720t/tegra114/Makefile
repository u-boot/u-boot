#
# Copyright (c) 2010-2013, NVIDIA CORPORATION.  All rights reserved.
#
# (C) Copyright 2000-2008
# Wolfgang Denk, DENX Software Engineering, wd@denx.de.
#
# This program is free software; you can redistribute it and/or modify it
# under the terms and conditions of the GNU General Public License,
# version 2, as published by the Free Software Foundation.
#
# This program is distributed in the hope it will be useful, but WITHOUT
# ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
# FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for
# more details.
#
# You should have received a copy of the GNU General Public License
# along with this program.  If not, see <http://www.gnu.org/licenses/>.
#

include $(TOPDIR)/config.mk

LIB	= $(obj)lib$(SOC).o

#COBJS-y	+= cpu.o t11x.o
COBJS-y	+= cpu.o

SRCS	:= $(COBJS-y:.o=.c)
OBJS	:= $(addprefix $(obj),$(COBJS-y))

all:	$(obj).depend $(LIB)

$(LIB):	$(OBJS)
	$(call cmd_link_o_target, $(OBJS))

#########################################################################

# defines $(obj).depend target
include $(SRCTREE)/rules.mk

sinclude $(obj).depend

#########################################################################
