#
# Copyright (C) 2009 Samsung Electronics
# Minkyu Kang <mk7.kang@samsung.com>
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

LIB	= $(obj)lib$(SOC).o

COBJS-y	+= clock.o power.o soc.o system.o pinmux.o tzpc.o

ifdef CONFIG_SPL_BUILD
COBJS-$(CONFIG_EXYNOS5)	+= clock_init_exynos5.o
COBJS-$(CONFIG_EXYNOS5)	+= dmc_common.o dmc_init_ddr3.o
COBJS-$(CONFIG_EXYNOS4210)+= dmc_init_exynos4.o clock_init_exynos4.o
COBJS-y	+= spl_boot.o
COBJS-y	+= lowlevel_init.o
endif

COBJS   := $(COBJS-y)
SRCS	:= $(COBJS:.o=.c)
OBJS	:= $(addprefix $(obj),$(COBJS))

all:	 $(obj).depend $(LIB)

$(LIB):	$(OBJS)
	$(call cmd_link_o_target, $(OBJS))

#########################################################################

# defines $(obj).depend target
include $(SRCTREE)/rules.mk

sinclude $(obj).depend

#########################################################################
