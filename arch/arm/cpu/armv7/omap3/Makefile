#
# (C) Copyright 2000-2003
# Wolfgang Denk, DENX Software Engineering, wd@denx.de.
#
# SPDX-License-Identifier:	GPL-2.0+
#

include $(TOPDIR)/config.mk

LIB	=  $(obj)lib$(SOC).o

SOBJS	:= lowlevel_init.o

COBJS	+= board.o
COBJS	+= clock.o
COBJS	+= mem.o
COBJS	+= sys_info.o
ifdef CONFIG_SPL_BUILD
COBJS-$(CONFIG_SPL_OMAP3_ID_NAND)	+= spl_id_nand.o
endif

COBJS-$(CONFIG_DRIVER_TI_EMAC)	+= emac.o
COBJS-$(CONFIG_EMIF4)	+= emif4.o
COBJS-$(CONFIG_SDRC)	+= sdrc.o
COBJS-$(CONFIG_USB_MUSB_AM35X)	+= am35x_musb.o

SRCS	:= $(SOBJS:.o=.S) $(COBJS:.o=.c)
OBJS	:= $(addprefix $(obj),$(COBJS) $(COBJS-y) $(SOBJS))

all:	 $(obj).depend $(LIB)

$(LIB):	$(OBJS)
	$(call cmd_link_o_target, $(OBJS))

#########################################################################

# defines $(obj).depend target
include $(SRCTREE)/rules.mk

sinclude $(obj).depend

#########################################################################
