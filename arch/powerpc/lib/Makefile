#
# (C) Copyright 2000-2006
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

LIB	= $(obj)lib$(ARCH).a

SOBJS-y	+= ppccache.o
SOBJS-y	+= ppcstring.o
SOBJS-y	+= ticks.o
SOBJS-y	+= reloc.o

COBJS-y	+= bat_rw.o
COBJS-y	+= board.o
COBJS-y	+= bootm.o
COBJS-$(CONFIG_BOOTCOUNT_LIMIT) += bootcount.o
COBJS-y	+= cache.o
COBJS-y	+= extable.o
COBJS-y	+= interrupts.o
COBJS-$(CONFIG_CMD_KGDB) += kgdb.o
COBJS-y	+= time.o

# Workaround for local bus unaligned access problems
# on MPC512x and MPC5200
ifdef CONFIG_MPC512X
$(obj)ppcstring.o: AFLAGS += -Dmemcpy=__memcpy
COBJS-y += memcpy_mpc5200.o
endif
ifdef CONFIG_MPC5200
$(obj)ppcstring.o: AFLAGS += -Dmemcpy=__memcpy
COBJS-y += memcpy_mpc5200.o
endif

COBJS	+= $(sort $(COBJS-y))

SRCS	:= $(SOBJS-y:.o=.S) $(COBJS-y:.o=.c)
OBJS	:= $(addprefix $(obj),$(SOBJS-y) $(COBJS-y))

$(LIB):	$(obj).depend $(OBJS)
	@if ! $(CROSS_COMPILE)readelf -S $(OBJS) | grep -q '\.fixup.*PROGBITS';\
	then \
		echo "ERROR: Your compiler doesn't generate .fixup sections!";\
		echo "       Upgrade to a recent toolchain."; \
		exit 1; \
	fi;
	$(AR) $(ARFLAGS) $@ $(OBJS)

#########################################################################

# defines $(obj).depend target
include $(SRCTREE)/rules.mk

sinclude $(obj).depend

#########################################################################
