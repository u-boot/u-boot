#
# (C) Copyright 2000-2007
# Wolfgang Denk, DENX Software Engineering, wd@denx.de.
#
# SPDX-License-Identifier:	GPL-2.0+
#

include $(TOPDIR)/config.mk

LIB	:= $(obj)libspi.o

# There are many options which enable SPI, so make this library available
COBJS-y += spi.o

COBJS-$(CONFIG_ALTERA_SPI) += altera_spi.o
COBJS-$(CONFIG_ANDES_SPI) += andes_spi.o
COBJS-$(CONFIG_ARMADA100_SPI) += armada100_spi.o
COBJS-$(CONFIG_ATMEL_DATAFLASH_SPI) += atmel_dataflash_spi.o
COBJS-$(CONFIG_ATMEL_SPI) += atmel_spi.o
COBJS-$(CONFIG_BFIN_SPI) += bfin_spi.o
COBJS-$(CONFIG_BFIN_SPI6XX) += bfin_spi6xx.o
COBJS-$(CONFIG_CF_SPI) += cf_spi.o
COBJS-$(CONFIG_CF_QSPI) += cf_qspi.o
COBJS-$(CONFIG_DAVINCI_SPI) += davinci_spi.o
COBJS-$(CONFIG_EXYNOS_SPI) += exynos_spi.o
COBJS-$(CONFIG_ICH_SPI) +=  ich.o
COBJS-$(CONFIG_KIRKWOOD_SPI) += kirkwood_spi.o
COBJS-$(CONFIG_MPC52XX_SPI) += mpc52xx_spi.o
COBJS-$(CONFIG_MPC8XXX_SPI) += mpc8xxx_spi.o
COBJS-$(CONFIG_MXC_SPI) += mxc_spi.o
COBJS-$(CONFIG_MXS_SPI) += mxs_spi.o
COBJS-$(CONFIG_OC_TINY_SPI) += oc_tiny_spi.o
COBJS-$(CONFIG_OMAP3_SPI) += omap3_spi.o
COBJS-$(CONFIG_SOFT_SPI) += soft_spi.o
COBJS-$(CONFIG_SH_SPI) += sh_spi.o
COBJS-$(CONFIG_FSL_ESPI) += fsl_espi.o
COBJS-$(CONFIG_FDT_SPI) += fdt_spi.o
COBJS-$(CONFIG_TEGRA20_SFLASH) += tegra20_sflash.o
COBJS-$(CONFIG_TEGRA20_SLINK) += tegra20_slink.o
COBJS-$(CONFIG_TEGRA114_SPI) += tegra114_spi.o
COBJS-$(CONFIG_XILINX_SPI) += xilinx_spi.o
COBJS-$(CONFIG_ZYNQ_SPI) += zynq_spi.o

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
