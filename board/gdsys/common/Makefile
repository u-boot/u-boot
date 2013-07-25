#
# (C) Copyright 2007
# Stefan Roese, DENX Software Engineering, sr@denx.de.
#
# SPDX-License-Identifier:	GPL-2.0+
#

include $(TOPDIR)/config.mk

ifneq ($(OBJTREE),$(SRCTREE))
$(shell mkdir -p $(obj)board/$(VENDOR)/common)
endif

LIB	= $(obj)lib$(VENDOR).o

COBJS-$(CONFIG_SYS_FPGA_COMMON) += fpga.o

COBJS-$(CONFIG_IO) += miiphybb.o
COBJS-$(CONFIG_IO64) += miiphybb.o
COBJS-$(CONFIG_IOCON) += osd.o mclink.o
COBJS-$(CONFIG_DLVISION_10G) += osd.o
COBJS-$(CONFIG_CONTROLCENTERD) += dp501.o

COBJS   := $(COBJS-y)
SOBJS   =

SRCS	:= $(SOBJS:.o=.S) $(COBJS:.o=.c)
OBJS	:= $(addprefix $(obj),$(COBJS))
SOBJS	:= $(addprefix $(obj),$(SOBJS))

$(LIB):	$(OBJS) $(SOBJS)
	$(call cmd_link_o_target, $(OBJS) $(SOBJS))

#########################################################################

# defines $(obj).depend target
include $(SRCTREE)/rules.mk

sinclude $(obj).depend

#########################################################################
