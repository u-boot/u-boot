#
# (C) Copyright 2002-2006
# Wolfgang Denk, DENX Software Engineering, wd@denx.de.
#
# SPDX-License-Identifier:	GPL-2.0+
#

include $(TOPDIR)/config.mk
include $(OBJTREE)/include/autoconf.mk

LIB				= libpost.o
GPLIB-$(CONFIG_HAS_POST)	+= libgenpost.o
COBJS-$(CONFIG_HAS_POST)	+= post.o
COBJS-$(CONFIG_POST_STD_LIST)	+= tests.o

SPLIB-$(CONFIG_HAS_POST) = drivers/libpostdrivers.o
SPLIB-$(CONFIG_HAS_POST) += $(shell if [ -d lib_$(ARCH) ]; then echo \
			    "lib_$(ARCH)/libpost$(ARCH).o"; fi)
SPLIB-$(CONFIG_HAS_POST) += $(shell if [ -d lib_$(ARCH)/fpu ]; then echo \
			    "lib_$(ARCH)/fpu/libpost$(ARCH)fpu.o"; fi)
SPLIB-$(CONFIG_HAS_POST) += $(shell if [ -d cpu/$(CPU) ]; then echo \
			    "cpu/$(CPU)/libpost$(CPU).o"; fi)
SPLIB-$(CONFIG_HAS_POST) += $(shell if [ -d board/$(BOARD) ]; then echo \
			    "board/$(BOARD)/libpost$(BOARD).o"; fi)

GPLIB	:= $(addprefix $(obj),$(GPLIB-y))
SPLIB	:= $(addprefix $(obj),$(SPLIB-y))
COBJS	:= $(COBJS-y)
SRCS	:= $(SOBJS:.o=.S) $(COBJS:.o=.c)
OBJS	:= $(addprefix $(obj),$(COBJS))
LIB	:= $(obj)$(LIB)

all:	$(LIB)

postdeps:
	@for lib in $(SPLIB-y) ; do \
		$(MAKE) -C `dirname $$lib` all ; \
	done

# generic POST library
$(GPLIB): $(obj).depend $(OBJS)
	$(call cmd_link_o_target, $(OBJS))

# specific POST libraries
$(SPLIB): $(obj).depend postdeps
	$(MAKE) -C $(dir $(subst $(obj),,$@))

# the POST lib archive
$(LIB): $(GPLIB) $(SPLIB)
	$(call cmd_link_o_target, $^)

#########################################################################

# defines $(obj).depend target
include $(SRCTREE)/rules.mk

sinclude $(obj).depend

#########################################################################
