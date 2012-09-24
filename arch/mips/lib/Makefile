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

LIB	= $(obj)lib$(ARCH).o

## Build a couple of necessary functions into a private libgcc
LIBGCC	= $(obj)libgcc.o
GLSOBJS	+= ashldi3.o
GLSOBJS	+= ashrdi3.o
GLSOBJS	+= lshrdi3.o
LGOBJS	:= $(addprefix $(obj),$(GLSOBJS))

SOBJS-y	+=

COBJS-y	+= board.o
ifeq ($(CONFIG_QEMU_MIPS),y)
COBJS-y	+= bootm_qemu_mips.o
else
COBJS-y	+= bootm.o
endif

SRCS	:= $(SOBJS-y:.o=.S) $(COBJS-y:.o=.c)
OBJS	:= $(addprefix $(obj),$(SOBJS-y) $(COBJS-y))

# Always build libmips.o
TARGETS	:= $(LIB)

# Build private libgcc only when asked for
ifdef USE_PRIVATE_LIBGCC
TARGETS	+= $(LIBGCC)
endif

all:	$(TARGETS)

$(LIB):	$(obj).depend $(OBJS)
	$(call cmd_link_o_target, $(OBJS))

$(LIBGCC): $(obj).depend $(LGOBJS)
	$(call cmd_link_o_target, $(LGOBJS))

#########################################################################

# defines $(obj).depend target
include $(SRCTREE)/rules.mk

sinclude $(obj).depend

#########################################################################
