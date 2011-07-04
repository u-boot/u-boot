#
# (C) Copyright 2006
# Wolfgang Denk, DENX Software Engineering, wd@denx.de.
#
# See file CREDITS for list of people who contributed to this
# project.
#
# This program is free software; you can redistribute it and/or
# modify it under the terms of the GNU General Public License as
# published by the Free Software Foundation; either version 2 of
# the License, or (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program; if not, write to the Free Software
# Foundation, Inc., 59 Temple Place, Suite 330, Boston,
# MA 02111-1307 USA
#

include $(TOPDIR)/config.mk

LIB	:= $(obj)libmmc.o

COBJS-$(CONFIG_ATMEL_MCI) += atmel_mci.o
COBJS-$(CONFIG_BFIN_SDH) += bfin_sdh.o
COBJS-$(CONFIG_DAVINCI_MMC) += davinci_mmc.o
COBJS-$(CONFIG_FSL_ESDHC) += fsl_esdhc.o
COBJS-$(CONFIG_GENERIC_MMC) += mmc.o
COBJS-$(CONFIG_GENERIC_ATMEL_MCI) += gen_atmel_mci.o
COBJS-$(CONFIG_MMC_SPI) += mmc_spi.o
COBJS-$(CONFIG_ARM_PL180_MMCI) += arm_pl180_mmci.o
COBJS-$(CONFIG_MV_SDHCI) += mv_sdhci.o
COBJS-$(CONFIG_MXC_MMC) += mxcmmc.o
COBJS-$(CONFIG_OMAP3_MMC) += omap3_mmc.o
COBJS-$(CONFIG_OMAP_HSMMC) += omap_hsmmc.o
COBJS-$(CONFIG_PXA_MMC) += pxa_mmc.o
COBJS-$(CONFIG_S5P_MMC) += s5p_mmc.o
COBJS-$(CONFIG_SDHCI) += sdhci.o
COBJS-$(CONFIG_SH_MMCIF) += sh_mmcif.o
COBJS-$(CONFIG_TEGRA2_MMC) += tegra2_mmc.o

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
