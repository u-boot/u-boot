#
# (C) Copyright 2000-2006
# Wolfgang Denk, DENX Software Engineering, wd@denx.de.
#
# SPDX-License-Identifier:	GPL-2.0+
#

include $(TOPDIR)/config.mk

LIB	= $(obj)lib$(SOC).o

COBJS-y	:= cpu.o \
	   reset.o \
	   timer.o

ifdef CONFIG_SPL_BUILD
COBJS-y	+= spl.o spl_boot.o
COBJS-$(CONFIG_SPEAR600) += spear600.o
COBJS-$(CONFIG_DDR_MT47H64M16) += spr600_mt47h64m16_3_333_cl5_psync.o
COBJS-$(CONFIG_DDR_MT47H32M16) += spr600_mt47h32m16_333_cl5_psync.o
COBJS-$(CONFIG_DDR_MT47H32M16) += spr600_mt47h32m16_37e_166_cl4_sync.o
COBJS-$(CONFIG_DDR_MT47H128M8) += spr600_mt47h128m8_3_266_cl5_async.o
endif

SRCS	:= $(START:.o=.S) $(COBJS-y:.o=.c)
OBJS	:= $(addprefix $(obj),$(COBJS-y))
START	:= $(addprefix $(obj),$(START))

all:	$(obj).depend $(LIB)

$(LIB):	$(OBJS)
	$(call cmd_link_o_target, $(OBJS))

#########################################################################

# defines $(obj).depend target
include $(SRCTREE)/rules.mk

sinclude $(obj).depend

#########################################################################
