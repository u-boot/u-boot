#
# Copyright (C) Freescale Semiconductor, Inc. 2006.
#
# SPDX-License-Identifier:	GPL-2.0+
#

include $(TOPDIR)/config.mk

LIB	= $(obj)lib$(BOARD).o

COBJS	:= $(BOARD).o pci.o fpga.o

SRCS	:= $(SOBJS:.o=.S) $(COBJS:.o=.c)
OBJS	:= $(addprefix $(obj),$(COBJS))
SOBJS	:= $(addprefix $(obj),$(SOBJS))

$(LIB):	$(obj).depend $(OBJS)
	$(call cmd_link_o_target, $(OBJS))
	@mkimage -T script -C none -n M7_script -d bootscript $(obj)bootscript.img

#########################################################################

include $(SRCTREE)/rules.mk

sinclude $(obj).depend

#########################################################################
