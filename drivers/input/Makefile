#
# (C) Copyright 2000-2007
# Wolfgang Denk, DENX Software Engineering, wd@denx.de.
#
# SPDX-License-Identifier:	GPL-2.0+
#

include $(TOPDIR)/config.mk

LIB	:= $(obj)libinput.o

COBJS-$(CONFIG_I8042_KBD) += i8042.o
COBJS-$(CONFIG_TEGRA_KEYBOARD) += tegra-kbc.o
COBJS-$(CONFIG_CROS_EC_KEYB) += cros_ec_keyb.o
ifdef CONFIG_PS2KBD
COBJS-y += keyboard.o pc_keyb.o
COBJS-$(CONFIG_PS2MULT) += ps2mult.o ps2ser.o
endif
COBJS-y += input.o
COBJS-$(CONFIG_OF_CONTROL) += key_matrix.o

COBJS	:= $(COBJS-y)
SRCS	:= $(COBJS:.o=.c)
OBJS	:= $(addprefix $(obj),$(COBJS))

all:	$(LIB)

$(LIB):	$(obj).depend $(OBJS)
	$(call cmd_link_o_target, $(OBJS))

#########################################################################

# defines $(obj).depend target
include $(SRCTREE)/rules.mk

sinclude $(obj).depend

#########################################################################
