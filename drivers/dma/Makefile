#
# (C) Copyright 2006
# Wolfgang Denk, DENX Software Engineering, wd@denx.de.
#
# SPDX-License-Identifier:	GPL-2.0+
#

include $(TOPDIR)/config.mk

LIB	:= $(obj)libdma.o

COBJS-$(CONFIG_FSLDMAFEC) += MCD_tasksInit.o MCD_dmaApi.o MCD_tasks.o
COBJS-$(CONFIG_APBH_DMA) += apbh_dma.o
COBJS-$(CONFIG_FSL_DMA) += fsl_dma.o
COBJS-$(CONFIG_OMAP3_DMA) += omap3_dma.o

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
