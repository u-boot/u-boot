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

LIB	:= $(obj)libmisc.o

COBJS-$(CONFIG_ALI152X) += ali512x.o
COBJS-$(CONFIG_DS4510)  += ds4510.o
COBJS-$(CONFIG_FSL_LAW) += fsl_law.o
COBJS-$(CONFIG_GPIO_LED) += gpio_led.o
COBJS-$(CONFIG_FSL_MC9SDZ60) += mc9sdz60.o
COBJS-$(CONFIG_NS87308) += ns87308.o
COBJS-$(CONFIG_PDSP188x) += pdsp188x.o
COBJS-$(CONFIG_STATUS_LED) += status_led.o
COBJS-$(CONFIG_TWL4030_LED) += twl4030_led.o
COBJS-$(CONFIG_PMIC) += pmic_core.o
COBJS-$(CONFIG_PMIC_FSL) += pmic_fsl.o
COBJS-$(CONFIG_PMIC_I2C) += pmic_i2c.o
COBJS-$(CONFIG_PMIC_SPI) += pmic_spi.o
COBJS-$(CONFIG_PMIC_MAX8998) += pmic_max8998.o

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
