#
# SPDX-License-Identifier:	GPL-2.0+
#

include $(TOPDIR)/config.mk

LIB 	:= $(obj)libbootcount.o

COBJS-y				+= bootcount.o
COBJS-$(CONFIG_AT91SAM9XE)	+= bootcount_at91.o
COBJS-$(CONFIG_BLACKFIN)	+= bootcount_blackfin.o
COBJS-$(CONFIG_SOC_DA8XX)	+= bootcount_davinci.o
COBJS-$(CONFIG_BOOTCOUNT_RAM)	+= bootcount_ram.o

COBJS	:= $(COBJS-y)
SRCS 	:= $(COBJS:.o=.c)
OBJS 	:= $(addprefix $(obj),$(COBJS))

$(LIB):	$(obj).depend $(OBJS)
	$(call cmd_link_o_target, $(OBJS))

#########################################################################

# defines $(obj).depend target
include $(SRCTREE)/rules.mk

sinclude $(obj).depend

########################################################################
