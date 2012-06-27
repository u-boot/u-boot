#
# Copyright (C) 2012  Nobuhiro Iwamatsu <nobuhiro.iwamatsu.yj@renesas.com>
#
# This program is free software; you can redistribute it and/or
# modify it under the terms of the GNU General Public License as
# published by the Free Software Foundation; either version 2 of
# the License.
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

include $(TOPDIR)/config.mk

LIB	= $(obj)lib$(BOARD).o

COBJS-y	+= armadillo-800eva.o
COBJS   := $(COBJS-y)

SRCS	:= $(COBJS:.o=.c)
OBJS	:= $(addprefix $(obj),$(COBJS))

$(LIB):	$(obj).depend $(OBJS)
	$(call cmd_link_o_target, $(OBJS))

clean:
	rm -f $(OBJS)

distclean:	clean
	rm -f $(LIB) core *.bak $(obj).depend

#########################################################################

# defines $(obj).depend target
include $(SRCTREE)/rules.mk

sinclude $(obj).depend

#########################################################################

