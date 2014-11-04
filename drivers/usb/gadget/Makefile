#
# (C) Copyright 2000-2007
# Wolfgang Denk, DENX Software Engineering, wd@denx.de.
#
# SPDX-License-Identifier:	GPL-2.0+
#

obj-$(CONFIG_USB_GADGET) += epautoconf.o config.o usbstring.o
obj-$(CONFIG_USB_ETHER) += epautoconf.o config.o usbstring.o

# new USB gadget layer dependencies
ifdef CONFIG_USB_GADGET
obj-$(CONFIG_USB_GADGET_ATMEL_USBA) += atmel_usba_udc.o
obj-$(CONFIG_USB_GADGET_S3C_UDC_OTG) += s3c_udc_otg.o
obj-$(CONFIG_USB_GADGET_S3C_UDC_OTG_PHY) += s3c_udc_otg_phy.o
obj-$(CONFIG_USB_GADGET_FOTG210) += fotg210.o
obj-$(CONFIG_CI_UDC)	+= ci_udc.o
obj-$(CONFIG_THOR_FUNCTION) += f_thor.o
obj-$(CONFIG_USBDOWNLOAD_GADGET) += g_dnl.o
obj-$(CONFIG_DFU_FUNCTION) += f_dfu.o
obj-$(CONFIG_USB_GADGET_MASS_STORAGE) += f_mass_storage.o
obj-$(CONFIG_CMD_FASTBOOT) += f_fastboot.o
endif
ifdef CONFIG_USB_ETHER
obj-y += ether.o
obj-$(CONFIG_USB_ETH_RNDIS) += rndis.o
obj-$(CONFIG_CI_UDC)	+= ci_udc.o
obj-$(CONFIG_CPU_PXA25X) += pxa25x_udc.o
else
# Devices not related to the new gadget layer depend on CONFIG_USB_DEVICE
ifdef CONFIG_USB_DEVICE
obj-y += core.o
obj-y += ep0.o
obj-$(CONFIG_DW_UDC) += designware_udc.o
obj-$(CONFIG_MPC885_FAMILY) += mpc8xx_udc.o
obj-$(CONFIG_CPU_PXA27X) += pxa27x_udc.o
endif
endif
