#
# (C) Copyright 2000-2008
# Wolfgang Denk, DENX Software Engineering, wd@denx.de.
#
# SPDX-License-Identifier:	GPL-2.0+
#

include $(TOPDIR)/config.mk

LIB	= $(obj)lib$(SOC).o

COBJS-$(CONFIG_AT91CAP9)	+= at91cap9_devices.o
COBJS-$(CONFIG_AT91SAM9260)	+= at91sam9260_devices.o
COBJS-$(CONFIG_AT91SAM9G20)	+= at91sam9260_devices.o
COBJS-$(CONFIG_AT91SAM9XE)	+= at91sam9260_devices.o
COBJS-$(CONFIG_AT91SAM9261)	+= at91sam9261_devices.o
COBJS-$(CONFIG_AT91SAM9G10)	+= at91sam9261_devices.o
COBJS-$(CONFIG_AT91SAM9263)	+= at91sam9263_devices.o
COBJS-$(CONFIG_AT91SAM9RL)	+= at91sam9rl_devices.o
COBJS-$(CONFIG_AT91SAM9M10G45)	+= at91sam9m10g45_devices.o
COBJS-$(CONFIG_AT91SAM9G45)	+= at91sam9m10g45_devices.o
COBJS-$(CONFIG_AT91SAM9N12)	+= at91sam9n12_devices.o
COBJS-$(CONFIG_AT91SAM9X5)	+= at91sam9x5_devices.o
COBJS-$(CONFIG_AT91_EFLASH)	+= eflash.o
COBJS-$(CONFIG_AT91_LED)	+= led.o
COBJS-y += clock.o
COBJS-y += cpu.o
COBJS-y	+= reset.o
COBJS-y	+= timer.o

ifndef CONFIG_SKIP_LOWLEVEL_INIT
SOBJS-y	:= lowlevel_init.o
endif

SRCS    := $(SOBJS-y:.o=.S) $(COBJS-y:.o=.c)
OBJS    := $(addprefix $(obj),$(SOBJS-y) $(COBJS-y))

all:	$(obj).depend $(LIB)

$(LIB):	$(OBJS)
	$(call cmd_link_o_target, $(OBJS))

#########################################################################

# defines $(obj).depend target
include $(SRCTREE)/rules.mk

sinclude $(obj).depend

#########################################################################
