#
# (C) Copyright 2011
# eInfochips Ltd. <www.einfochips.com>
# Written-by: Ajay Bhargav <ajay.bhargav@einfochips.com>
#
# Based on Aspenite:
# (C) Copyright 2010
# Marvell Semiconductor <www.marvell.com>
# Written-by: Prafulla Wadaskar <prafulla@marvell.com>
# Contributor: Mahavir Jain <mjain@marvell.com>
#
# SPDX-License-Identifier:	GPL-2.0+
#

include $(TOPDIR)/config.mk

LIB     = $(obj)lib$(BOARD).o

COBJS	:= gplugd.o

SRCS	:= $(SOBJS:.o=.S) $(COBJS:.o=.c)
OBJS	:= $(addprefix $(obj),$(COBJS))
SOBJS	:= $(addprefix $(obj),$(SOBJS))

$(LIB):	$(obj).depend $(OBJS) $(SOBJS)
	$(call cmd_link_o_target, $(OBJS) $(SOBJS))

#########################################################################

# defines $(obj).depend target
include $(SRCTREE)/rules.mk

sinclude $(obj).depend

#########################################################################
