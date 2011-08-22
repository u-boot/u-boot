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
COBJS-$(CONFIG_KIRKWOOD_GPIO)	+= kw_gpio.o
COBJS-$(CONFIG_MARVELL_GPIO)	+= mvgpio.o
COBJS-$(CONFIG_MARVELL_MFP)	+= mvmfp.o
COBJS-$(CONFIG_MXC_GPIO)	+= mxc_gpio.o
COBJS-$(CONFIG_PCA953X)		+= pca953x.o
COBJS-$(CONFIG_S5P)		+= s5p_gpio.o
COBJS-$(CONFIG_TEGRA2_GPIO)	+= tegra2_gpio.o
COBJS-$(CONFIG_DA8XX_GPIO)	+= da8xx_gpio.o

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
