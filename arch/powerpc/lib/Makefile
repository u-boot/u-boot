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

## Build a couple of necessary functions into a private libgcc
LIBGCC	= $(obj)libgcc.o
GLSOBJS	+= _ashldi3.o
GLSOBJS	+= _ashrdi3.o
GLSOBJS	+= _lshrdi3.o
LGOBJS	:= $(addprefix $(obj),$(GLSOBJS)) \
	   $(addprefix $(obj),$(GLCOBJS))

## But only build it if the user asked for it
ifdef USE_PRIVATE_LIBGCC
TARGETS	+= $(LIBGCC)
endif

LIB	= $(obj)lib$(ARCH).o

MINIMAL=

ifdef CONFIG_SPL_BUILD
ifdef CONFIG_SPL_INIT_MINIMAL
MINIMAL=y
endif
endif

ifdef MINIMAL
COBJS-y += cache.o
else

SOBJS-y	+= ppcstring.o

SOBJS-y	+= ppccache.o
SOBJS-y	+= ticks.o
SOBJS-y	+= reloc.o

COBJS-$(CONFIG_BAT_RW) += bat_rw.o
ifndef CONFIG_SPL_BUILD
COBJS-y	+= board.o
endif
COBJS-y	+= bootm.o
COBJS-y	+= cache.o
COBJS-y	+= extable.o
COBJS-y	+= interrupts.o
COBJS-$(CONFIG_CMD_KGDB) += kgdb.o
COBJS-${CONFIG_CMD_IDE} += ide.o
COBJS-y	+= time.o

# Don't include the MPC5xxx special memcpy into the
# SPL U-Boot image. memcpy is used in the SPL NOR
# flash driver. And we need the real, fast memcpy
# here. We have no problems with unaligned access.
ifndef CONFIG_SPL_BUILD
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
endif

endif # not minimal

ifdef CONFIG_SPL_BUILD
COBJS-$(CONFIG_SPL_FRAMEWORK) += spl.o
endif

COBJS	+= $(sort $(COBJS-y))

SRCS	:= $(GLSOBJS:.o=.S) $(GLCOBJS:.o=.c) \
	   $(SOBJS-y:.o=.S) $(COBJS-y:.o=.c)
OBJS	:= $(addprefix $(obj),$(SOBJS-y) $(COBJS-y))

TARGETS += $(LIB)

all: $(TARGETS)

$(LIB):	$(obj).depend $(OBJS)
	$(call cmd_link_o_target, $(OBJS))

$(LIBGCC): $(obj).depend $(LGOBJS)
	$(call cmd_link_o_target, $(LGOBJS))

#########################################################################

# defines $(obj).depend target
include $(SRCTREE)/rules.mk

sinclude $(obj).depend

#########################################################################
