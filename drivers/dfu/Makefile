#
# Copyright (C) 2012 Samsung Electronics
# Lukasz Majewski <l.majewski@samsung.com>
#
# SPDX-License-Identifier:	GPL-2.0+
#

include $(TOPDIR)/config.mk

LIB	= $(obj)libdfu.o

COBJS-$(CONFIG_DFU_FUNCTION) += dfu.o
COBJS-$(CONFIG_DFU_MMC) += dfu_mmc.o
COBJS-$(CONFIG_DFU_NAND) += dfu_nand.o
COBJS-$(CONFIG_DFU_RAM) += dfu_ram.o

SRCS    := $(COBJS-y:.o=.c)
OBJS	:= $(addprefix $(obj),$(COBJS-y))

$(LIB):	$(obj).depend $(OBJS)
	$(call cmd_link_o_target, $(OBJS))

#########################################################################

# defines $(obj).depend target
include $(SRCTREE)/rules.mk

sinclude $(obj).depend

#########################################################################
