#
# (C) Copyright 2002-2006
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
include $(TOPDIR)/include/autoconf.mk

LIB				= libpost.a
GPLIB-$(CONFIG_HAS_POST)	+= libgenpost.a
COBJS-$(CONFIG_HAS_POST)	+= post.o tests.o

SPLIB-$(CONFIG_HAS_POST) = drivers/libpostdrivers.a
SPLIB-$(CONFIG_HAS_POST) += $(shell if [ -d lib_$(ARCH) ]; then echo \
			    "lib_$(ARCH)/libpost$(ARCH).a"; fi)
SPLIB-$(CONFIG_HAS_POST) += $(shell if [ -d lib_$(ARCH)/fpu ]; then echo \
			    "lib_$(ARCH)/fpu/libpost$(ARCH)fpu.a"; fi)
SPLIB-$(CONFIG_HAS_POST) += $(shell if [ -d cpu/$(CPU) ]; then echo \
			    "cpu/$(CPU)/libpost$(CPU).a"; fi)
SPLIB-$(CONFIG_HAS_POST) += $(shell if [ -d board/$(BOARD) ]; then echo \
			    "board/$(BOARD)/libpost$(BOARD).a"; fi)

GPLIB	:= $(GPLIB-y)
SPLIB	:= $(SPLIB-y)
COBJS	:= $(COBJS-y)
SRCS	:= $(SOBJS:.o=.S) $(COBJS:.o=.c)
OBJS	:= $(addprefix $(obj),$(COBJS))
LIB	:= $(obj)$(LIB)

all:	$(LIB)

# generic POST library
$(GPLIB): $(obj).depend $(OBJS)
	$(AR) $(ARFLAGS) $@ $(OBJS)

# specific POST libraries
$(SPLIB): $(obj).depend
	$(MAKE) -C $(dir $(subst $(obj),,$@))

# the POST lib archive
$(LIB): $(GPLIB) $(SPLIB)
	(echo create $(LIB); for lib in $(GPLIB) $(SPLIB) ; \
	 do echo addlib $$lib; done; echo save) \
	| $(AR) -M

#########################################################################

# defines $(obj).depend target
include $(SRCTREE)/rules.mk

sinclude $(obj).depend

#########################################################################
