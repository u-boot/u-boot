#
# (C) Copyright 2002-2006
# Wolfgang Denk, DENX Software Engineering, wd@denx.de.
#
# SPDX-License-Identifier:	GPL-2.0+
#

include $(TOPDIR)/config.mk

LIB	= $(obj)lib$(ARCH).o
LIBGCC	= $(obj)libgcc.o

GLSOBJS	+= _ashldi3.o
GLSOBJS	+= _ashrdi3.o
GLSOBJS	+= _divsi3.o
GLSOBJS	+= _lshrdi3.o
GLSOBJS	+= _modsi3.o
GLSOBJS	+= _udivsi3.o
GLSOBJS	+= _umodsi3.o

GLCOBJS	+= div0.o

SOBJS-y += crt0.o

ifndef CONFIG_SPL_BUILD
SOBJS-y += relocate.o
ifndef CONFIG_SYS_GENERIC_BOARD
COBJS-y	+= board.o
endif
COBJS-y += sections.o

COBJS-$(CONFIG_OF_LIBFDT) += bootm-fdt.o
COBJS-$(CONFIG_CMD_BOOTM) += bootm.o
COBJS-$(CONFIG_SYS_L2_PL310) += cache-pl310.o
SOBJS-$(CONFIG_USE_ARCH_MEMSET) += memset.o
SOBJS-$(CONFIG_USE_ARCH_MEMCPY) += memcpy.o
else
COBJS-$(CONFIG_SPL_FRAMEWORK) += spl.o
endif

COBJS-y	+= interrupts.o
COBJS-y	+= reset.o

COBJS-y	+= cache.o
COBJS-y	+= cache-cp15.o

SRCS	:= $(GLSOBJS:.o=.S) $(GLCOBJS:.o=.c) \
	   $(SOBJS-y:.o=.S) $(COBJS-y:.o=.c)
OBJS	:= $(addprefix $(obj),$(SOBJS-y) $(COBJS-y))
LGOBJS	:= $(addprefix $(obj),$(GLSOBJS)) \
	   $(addprefix $(obj),$(GLCOBJS))

# Always build libarm.o
TARGETS	:= $(LIB)

# Build private libgcc only when asked for
ifdef USE_PRIVATE_LIBGCC
TARGETS	+= $(LIBGCC)
endif

# For EABI conformant tool chains, provide eabi_compat()
ifneq (,$(findstring -mabi=aapcs-linux,$(PLATFORM_CPPFLAGS)))
TARGETS	+= $(obj)eabi_compat.o
endif

all:	$(TARGETS)

$(LIB):	$(obj).depend $(OBJS)
	$(call cmd_link_o_target, $(OBJS))

$(LIBGCC): $(obj).depend $(LGOBJS)
	$(call cmd_link_o_target, $(LGOBJS))

#########################################################################

# defines $(obj).depend target
include $(SRCTREE)/rules.mk

sinclude $(obj).depend

#########################################################################
