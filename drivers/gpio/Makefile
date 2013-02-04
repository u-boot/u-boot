#
# Copyright 2000-2008
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

LIB 	:= $(obj)libgpio.o

COBJS-$(CONFIG_AT91_GPIO)	+= at91_gpio.o
COBJS-$(CONFIG_INTEL_ICH6_GPIO)	+= intel_ich6_gpio.o
COBJS-$(CONFIG_KIRKWOOD_GPIO)	+= kw_gpio.o
COBJS-$(CONFIG_MARVELL_GPIO)	+= mvgpio.o
COBJS-$(CONFIG_MARVELL_MFP)	+= mvmfp.o
COBJS-$(CONFIG_MXC_GPIO)	+= mxc_gpio.o
COBJS-$(CONFIG_MXS_GPIO)	+= mxs_gpio.o
COBJS-$(CONFIG_PCA953X)		+= pca953x.o
COBJS-$(CONFIG_PCA9698)		+= pca9698.o
COBJS-$(CONFIG_S5P)		+= s5p_gpio.o
COBJS-$(CONFIG_SANDBOX_GPIO)	+= sandbox.o
COBJS-$(CONFIG_SPEAR_GPIO)	+= spear_gpio.o
COBJS-$(CONFIG_TEGRA_GPIO)	+= tegra_gpio.o
COBJS-$(CONFIG_DA8XX_GPIO)	+= da8xx_gpio.o
COBJS-$(CONFIG_ALTERA_PIO)	+= altera_pio.o
COBJS-$(CONFIG_MPC83XX_GPIO)	+= mpc83xx_gpio.o
COBJS-$(CONFIG_SH_GPIO_PFC)	+= sh_pfc.o
COBJS-$(CONFIG_OMAP_GPIO)	+= omap_gpio.o
COBJS-$(CONFIG_DB8500_GPIO)	+= db8500_gpio.o
COBJS-$(CONFIG_BCM2835_GPIO)	+= bcm2835_gpio.o
COBJS-$(CONFIG_S3C2440_GPIO)	+= s3c2440_gpio.o

COBJS	:= $(COBJS-y)
SRCS 	:= $(COBJS:.o=.c)
OBJS 	:= $(addprefix $(obj),$(COBJS))

all:	$(LIB)

$(LIB):	$(obj).depend $(OBJS)
	$(call cmd_link_o_target, $(OBJS))


#########################################################################

# defines $(obj).depend target
include $(SRCTREE)/rules.mk

sinclude $(obj).depend

########################################################################
