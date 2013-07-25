#
# (C) Copyright 2000-2007
# Wolfgang Denk, DENX Software Engineering, wd@denx.de.
#
# SPDX-License-Identifier:	GPL-2.0+
#

include $(TOPDIR)/config.mk

LIB	:= $(obj)libmisc.o

COBJS-$(CONFIG_ALI152X) += ali512x.o
COBJS-$(CONFIG_DS4510)  += ds4510.o
COBJS-$(CONFIG_CBMEM_CONSOLE) += cbmem_console.o
COBJS-$(CONFIG_CROS_EC) += cros_ec.o
COBJS-$(CONFIG_CROS_EC_LPC) += cros_ec_lpc.o
COBJS-$(CONFIG_CROS_EC_I2C) += cros_ec_i2c.o
COBJS-$(CONFIG_CROS_EC_SPI) += cros_ec_spi.o
COBJS-$(CONFIG_FSL_IIM) += fsl_iim.o
COBJS-$(CONFIG_GPIO_LED) += gpio_led.o
COBJS-$(CONFIG_FSL_MC9SDZ60) += mc9sdz60.o
COBJS-$(CONFIG_MXC_OCOTP) += mxc_ocotp.o
COBJS-$(CONFIG_NS87308) += ns87308.o
COBJS-$(CONFIG_PDSP188x) += pdsp188x.o
COBJS-$(CONFIG_STATUS_LED) += status_led.o
COBJS-$(CONFIG_TWL4030_LED) += twl4030_led.o

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
