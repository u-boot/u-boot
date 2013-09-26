#
# (C) Copyright 2006
# Wolfgang Denk, DENX Software Engineering, wd@denx.de.
#
# SPDX-License-Identifier:	GPL-2.0+
#

include $(TOPDIR)/config.mk

LIB	:= $(obj)libspi_flash.o

ifdef CONFIG_SPL_BUILD
COBJS-$(CONFIG_SPL_SPI_LOAD)	+= spi_spl_load.o
COBJS-$(CONFIG_SPL_SPI_BOOT)	+= fsl_espi_spl.o
endif

ifdef CONFIG_CMD_SF
COBJS-y        += sf.o
endif
COBJS-$(CONFIG_SPI_FLASH) += sf_probe.o sf_ops.o
COBJS-$(CONFIG_SPI_FRAM_RAMTRON) += ramtron.o
COBJS-$(CONFIG_SPI_M95XXX) += eeprom_m95xxx.o

COBJS	:= $(COBJS-y)
SRCS	:= $(COBJS:.o=.c)
OBJS	:= $(addprefix $(obj),$(COBJS))

all:	$(LIB)

$(LIB): $(obj).depend $(OBJS)
	$(call cmd_link_o_target, $(OBJS))

#########################################################################

# defines $(obj).depend target
include $(SRCTREE)/rules.mk

sinclude $(obj).depend

#########################################################################
