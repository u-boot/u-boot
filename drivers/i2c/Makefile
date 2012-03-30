#
# (C) Copyright 2000-2007
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

LIB	:= $(obj)libi2c.o

COBJS-$(CONFIG_BFIN_TWI_I2C) += bfin-twi_i2c.o
COBJS-$(CONFIG_DRIVER_DAVINCI_I2C) += davinci_i2c.o
COBJS-$(CONFIG_FSL_I2C) += fsl_i2c.o
COBJS-$(CONFIG_I2C_MVTWSI) += mvtwsi.o
COBJS-$(CONFIG_I2C_MV) += mv_i2c.o
COBJS-$(CONFIG_I2C_MXC) += mxc_i2c.o
COBJS-$(CONFIG_I2C_MXS) += mxs_i2c.o
COBJS-$(CONFIG_DRIVER_OMAP1510_I2C) += omap1510_i2c.o
COBJS-$(CONFIG_DRIVER_OMAP24XX_I2C) += omap24xx_i2c.o
COBJS-$(CONFIG_DRIVER_OMAP34XX_I2C) += omap24xx_i2c.o
COBJS-$(CONFIG_PCA9564_I2C) += pca9564_i2c.o
COBJS-$(CONFIG_PPC4XX_I2C) += ppc4xx_i2c.o
COBJS-$(CONFIG_DRIVER_S3C24X0_I2C) += s3c24x0_i2c.o
COBJS-$(CONFIG_S3C44B0_I2C) += s3c44b0_i2c.o
COBJS-$(CONFIG_SOFT_I2C) += soft_i2c.o
COBJS-$(CONFIG_SPEAR_I2C) += spr_i2c.o
COBJS-$(CONFIG_TEGRA_I2C) += tegra_i2c.o
COBJS-$(CONFIG_TSI108_I2C) += tsi108_i2c.o
COBJS-$(CONFIG_U8500_I2C) += u8500_i2c.o
COBJS-$(CONFIG_SH_I2C) += sh_i2c.o

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
