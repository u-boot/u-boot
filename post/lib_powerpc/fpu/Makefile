#
# (C) Copyright 2007
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

LIB	= libpost$(ARCH)fpu.a

COBJS-$(CONFIG_HAS_POST)	+= fpu.o 20001122-1.o 20010114-2.o 20010226-1.o 980619-1.o
COBJS-$(CONFIG_HAS_POST)	+= acc1.o compare-fp-1.o mul-subnormal-single-1.o

include $(TOPDIR)/post/rules.mk

CFLAGS := $(shell echo $(CFLAGS) | sed s/-msoft-float//)
CFLAGS += -mhard-float -fkeep-inline-functions
