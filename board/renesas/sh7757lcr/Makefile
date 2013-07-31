#
# Copyright (C) 2011  Yoshihiro Shimoda <yoshihiro.shimoda.uh@renesas.com>
#
# SPDX-License-Identifier:	GPL-2.0+

include $(TOPDIR)/config.mk

LIB	= $(obj)lib$(BOARD).o

COBJS	:= sh7757lcr.o spi-boot.o
SOBJS	:= lowlevel_init.o

$(LIB):	$(obj).depend $(COBJS) $(SOBJS)
	$(call cmd_link_o_target, $(COBJS) $(SOBJS))

#########################################################################

# defines $(obj).depend target
include $(SRCTREE)/rules.mk

sinclude $(obj).depend

#########################################################################
