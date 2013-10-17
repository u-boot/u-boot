#
# Copyright (C) 2012 Samsung Electronics
# R. Chandrasekar <rcsekar@samsung.com>
#
# SPDX-License-Identifier:	GPL-2.0+
#

include $(TOPDIR)/config.mk

LIB	:= $(obj)libsound.o

COBJS-$(CONFIG_SOUND)	+= sound.o
COBJS-$(CONFIG_I2S)	+= samsung-i2s.o
COBJS-$(CONFIG_SOUND_WM8994)	+= wm8994.o
COBJS-$(CONFIG_SOUND_MAX98095)	+= max98095.o

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

#
