#
# (C) Copyright 2011
# eInfochips Ltd. <www.einfochips.com>
# Written-by: Ajay Bhargav <ajay.bhargav@einfochips.com>
#
# Based on Aspenite:
# (C) Copyright 2010
# Marvell Semiconductor <www.marvell.com>
# Written-by: Prafulla Wadaskar <prafulla@marvell.com>
# Contributor: Mahavir Jain <mjain@marvell.com>
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
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	 See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program; if not, write to the Free Software
# Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston,
# MA 02110-1301 USA
#

include $(TOPDIR)/config.mk

LIB     = $(obj)lib$(BOARD).o

COBJS	:= gplugd.o

SRCS	:= $(SOBJS:.o=.S) $(COBJS:.o=.c)
OBJS	:= $(addprefix $(obj),$(COBJS))
SOBJS	:= $(addprefix $(obj),$(SOBJS))

$(LIB):	$(obj).depend $(OBJS) $(SOBJS)
	$(call cmd_link_o_target, $(OBJS) $(SOBJS))

#########################################################################

# defines $(obj).depend target
include $(SRCTREE)/rules.mk

sinclude $(obj).depend

#########################################################################
