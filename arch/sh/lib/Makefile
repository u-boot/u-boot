#
# Copyright (c) 2007
# Nobuhiro Iwamatsu <iwamatsu@nigauri.org>
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
LIBGCC	= $(obj)libgcc.o

SOBJS-y	+=
GLSOBJS	+= ashiftrt.o
GLSOBJS	+= ashiftlt.o
GLSOBJS	+= lshiftrt.o
GLSOBJS	+= ashldi3.o
GLSOBJS	+= ashrsi3.o
GLSOBJS	+= lshrdi3.o
GLSOBJS	+= movmem.o

COBJS-y	+= board.o
COBJS-y	+= bootm.o
ifeq ($(CONFIG_SH2),y)
COBJS-y	+= time_sh2.o
else
COBJS-y	+= time.o
endif
ifeq ($(CONFIG_CMD_SH_ZIMAGEBOOT),y)
COBJS-y += zimageboot.o
endif

SRCS	:= $(SOBJS-y:.o=.S) $(COBJS-y:.o=.c)
OBJS	:= $(addprefix $(obj),$(SOBJS-y) $(COBJS-y))
LGOBJS	:= $(addprefix $(obj),$(GLSOBJS)) \
	   $(addprefix $(obj),$(GLCOBJS))

# Always build libsh.o
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
