#
# (C) Copyright 2000-2007
# Wolfgang Denk, DENX Software Engineering, wd@denx.de.
#
# SPDX-License-Identifier:	GPL-2.0+
#

include $(TOPDIR)/config.mk

LIB	:= $(obj)libusb_host.o

# ohci
COBJS-$(CONFIG_USB_OHCI_NEW) += ohci-hcd.o
COBJS-$(CONFIG_USB_ATMEL) += ohci-at91.o
COBJS-$(CONFIG_USB_OHCI_DA8XX) += ohci-da8xx.o
COBJS-$(CONFIG_USB_ISP116X_HCD) += isp116x-hcd.o
COBJS-$(CONFIG_USB_R8A66597_HCD) += r8a66597-hcd.o
COBJS-$(CONFIG_USB_SL811HS) += sl811-hcd.o
COBJS-$(CONFIG_USB_OHCI_S3C24XX) += ohci-s3c24xx.o

# echi
COBJS-$(CONFIG_USB_EHCI) += ehci-hcd.o
COBJS-$(CONFIG_USB_EHCI_ARMADA100) += ehci-armada100.o utmi-armada100.o
COBJS-$(CONFIG_USB_EHCI_ATMEL) += ehci-atmel.o
ifdef CONFIG_MPC512X
COBJS-$(CONFIG_USB_EHCI_FSL) += ehci-mpc512x.o
else
COBJS-$(CONFIG_USB_EHCI_FSL) += ehci-fsl.o
endif
COBJS-$(CONFIG_USB_EHCI_FARADAY) += ehci-faraday.o
COBJS-$(CONFIG_USB_EHCI_EXYNOS) += ehci-exynos.o
COBJS-$(CONFIG_USB_EHCI_MXC) += ehci-mxc.o
COBJS-$(CONFIG_USB_EHCI_MXS) += ehci-mxs.o
COBJS-$(CONFIG_USB_EHCI_MX5) += ehci-mx5.o
COBJS-$(CONFIG_USB_EHCI_MX6) += ehci-mx6.o
COBJS-$(CONFIG_USB_EHCI_OMAP) += ehci-omap.o
COBJS-$(CONFIG_USB_EHCI_PPC4XX) += ehci-ppc4xx.o
COBJS-$(CONFIG_USB_EHCI_IXP4XX) += ehci-ixp.o
COBJS-$(CONFIG_USB_EHCI_MARVELL) += ehci-marvell.o
COBJS-$(CONFIG_USB_EHCI_PCI) += ehci-pci.o
COBJS-$(CONFIG_USB_EHCI_SPEAR) += ehci-spear.o
COBJS-$(CONFIG_USB_EHCI_TEGRA) += ehci-tegra.o
COBJS-$(CONFIG_USB_EHCI_VCT) += ehci-vct.o

# xhci
COBJS-$(CONFIG_USB_XHCI) += xhci.o xhci-mem.o xhci-ring.o
COBJS-$(CONFIG_USB_XHCI_EXYNOS) += xhci-exynos5.o
COBJS-$(CONFIG_USB_XHCI_OMAP) += xhci-omap.o

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
