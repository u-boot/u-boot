#
# (C) Copyright 2007-2008 Netstal Maschinen AG
# Niklaus Giger (ng@netstal.com)
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

LIB	= $(obj)lib$(BOARD).a

# NOBJS : Netstal common objects
NOBJS	= fixed_sdram.o nm_bsp.o
COBJS	= $(BOARD).o
SOBJS	=

SRCS	:= $(SOBJS:.o=.S) $(COBJS:.o=.c) $(addprefix ../common/,$(NOBJS:.o=.c))
OBJS	:= $(addprefix $(obj),$(COBJS))
NOBJS	:= $(addprefix $(obj)../common/,$(NOBJS))
SOBJS	:= $(addprefix $(obj),$(SOBJS))

$(LIB):	$(OBJS) $(SOBJS) $(NOBJS)
	$(AR) $(ARFLAGS) $@ $(OBJS) $(SOBJS) $(NOBJS)

clean:
	rm -f $(SOBJS) $(OBJS)

distclean:	clean
	rm -f $(LIB) core *.bak $(obj).depend

#########################################################################

# defines $(obj).depend target
include $(SRCTREE)/rules.mk

sinclude $(obj).depend

#########################################################################
