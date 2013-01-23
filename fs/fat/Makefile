#
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

LIB	= $(obj)libfat.o

AOBJS	=
COBJS-$(CONFIG_FS_FAT)	:= fat.o
COBJS-$(CONFIG_FAT_WRITE):= fat_write.o

ifndef CONFIG_SPL_BUILD
COBJS-$(CONFIG_FS_FAT)	+= file.o
endif

SRCS	:= $(AOBJS:.o=.S) $(COBJS-y:.o=.c)
OBJS	:= $(addprefix $(obj),$(AOBJS) $(COBJS-y))

all:	$(LIB) $(AOBJS)

$(LIB):	$(obj).depend $(OBJS)
	$(call cmd_link_o_target, $(OBJS))

# SEE README.arm-unaligned-accesses
$(obj)file.o: CFLAGS += $(PLATFORM_NO_UNALIGNED)

#########################################################################

# defines $(obj).depend target
include $(SRCTREE)/rules.mk

sinclude $(obj).depend

#########################################################################
