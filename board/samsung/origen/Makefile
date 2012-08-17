#
# Copyright (C) 2011 Samsung Electronics
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

LIB	= $(obj)lib$(BOARD).o

SOBJS	:= mem_setup.o
SOBJS	+= lowlevel_init.o

ifndef CONFIG_SPL_BUILD
COBJS	+= origen.o
endif

ifdef CONFIG_SPL_BUILD
COBJS	+= mmc_boot.o
endif

SRCS	:= $(SOBJS:.o=.S) $(COBJS:.o=.c)
OBJS	:= $(addprefix $(obj),$(COBJS) $(SOBJS))

ALL	+=$(obj).depend $(LIB)

ifdef CONFIG_SPL_BUILD
ALL	+= $(OBJTREE)/tools/mk$(BOARD)spl
endif

all:	$(ALL)

$(LIB):	$(OBJS)
	$(call cmd_link_o_target, $(OBJS))

ifdef CONFIG_SPL_BUILD
$(OBJTREE)/tools/mk$(BOARD)spl:	tools/mkv310_image.c
	$(HOSTCC) tools/mkv310_image.c -o $(OBJTREE)/tools/mk$(BOARD)spl
endif

#########################################################################

# defines $(obj).depend target
include $(SRCTREE)/rules.mk

sinclude $(obj).depend

#########################################################################
