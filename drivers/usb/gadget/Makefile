#
# (C) Copyright 2000-2007
# Wolfgang Denk, DENX Software Engineering, wd@denx.de.
#
# SPDX-License-Identifier:	GPL-2.0+
#

include $(TOPDIR)/config.mk

LIB	:= $(obj)libusb_gadget.o

# if defined(CONFIG_USB_GADGET) || defined(CONFIG_USB_ETHER)
#   Everytime you forget how crufty makefiles can get things like
#   this remind you...
ifneq (,$(CONFIG_USB_GADGET)$(CONFIG_USB_ETHER))
COBJS-y += epautoconf.o config.o usbstring.o
endif

# new USB gadget layer dependencies
ifdef CONFIG_USB_GADGET
COBJS-$(CONFIG_USB_GADGET_ATMEL_USBA) += atmel_usba_udc.o
COBJS-$(CONFIG_USB_GADGET_S3C_UDC_OTG) += s3c_udc_otg.o
COBJS-$(CONFIG_USB_GADGET_FOTG210) += fotg210.o
COBJS-$(CONFIG_USBDOWNLOAD_GADGET) += g_dnl.o
COBJS-$(CONFIG_DFU_FUNCTION) += f_dfu.o
COBJS-$(CONFIG_USB_GADGET_MASS_STORAGE) += f_mass_storage.o
endif
ifdef CONFIG_USB_ETHER
COBJS-y += ether.o
COBJS-$(CONFIG_USB_ETH_RNDIS) += rndis.o
COBJS-$(CONFIG_MV_UDC)	+= mv_udc.o
COBJS-$(CONFIG_CPU_PXA25X) += pxa25x_udc.o
else
# Devices not related to the new gadget layer depend on CONFIG_USB_DEVICE
ifdef CONFIG_USB_DEVICE
COBJS-y += core.o
COBJS-y += ep0.o
COBJS-$(CONFIG_DW_UDC) += designware_udc.o
COBJS-$(CONFIG_OMAP1510) += omap1510_udc.o
COBJS-$(CONFIG_OMAP1610) += omap1510_udc.o
COBJS-$(CONFIG_MPC885_FAMILY) += mpc8xx_udc.o
COBJS-$(CONFIG_CPU_PXA27X) += pxa27x_udc.o
endif
endif

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
