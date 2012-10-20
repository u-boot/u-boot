#
# h2200 Support
#
# Copyright (C) 2012 Lukasz Dalek <luk0104@gmail.com>
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

LIB	= $(obj)lib$(BOARD).o

COBJS	:= h2200.o

SRCS	:= $(COBJS:.o=.c) h2200-header.S
OBJS	:= $(addprefix $(obj),$(COBJS))

all: $(LIB) $(obj)h2200-header.bin

$(obj)h2200-header.o: h2200-header.S
	$(CC) $(CFLAGS) -c -o $@ $<

$(obj)h2200-header.bin: $(obj)h2200-header.o
	$(OBJCOPY) -O binary $< $@

$(LIB):	$(obj).depend $(OBJS)
	$(call cmd_link_o_target, $(OBJS))

#########################################################################

# defines $(obj).depend target
include $(SRCTREE)/rules.mk

sinclude $(obj).depend

#########################################################################
