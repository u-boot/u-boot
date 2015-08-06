#
# (C) Copyright 2000-2007
# Wolfgang Denk, DENX Software Engineering, wd@denx.de.
#
# SPDX-License-Identifier:	GPL-2.0+
#

ifdef CONFIG_DM_USB
obj-$(CONFIG_CMD_USB) += usb-uclass.o
obj-$(CONFIG_SANDBOX) += usb-sandbox.o
endif

# ohci
obj-$(CONFIG_USB_OHCI_NEW) += ohci-hcd.o
obj-$(CONFIG_USB_ATMEL) += ohci-at91.o
obj-$(CONFIG_USB_OHCI_DA8XX) += ohci-da8xx.o
obj-$(CONFIG_USB_ISP116X_HCD) += isp116x-hcd.o
obj-$(CONFIG_USB_R8A66597_HCD) += r8a66597-hcd.o
obj-$(CONFIG_USB_SL811HS) += sl811-hcd.o
obj-$(CONFIG_USB_OHCI_S3C24XX) += ohci-s3c24xx.o
obj-$(CONFIG_USB_OHCI_EP93XX) += ohci-ep93xx.o
obj-$(CONFIG_USB_OHCI_SUNXI) += ohci-sunxi.o
obj-$(CONFIG_USB_OHCI_LPC32XX) += ohci-lpc32xx.o

# echi
obj-$(CONFIG_USB_EHCI) += ehci-hcd.o
obj-$(CONFIG_USB_EHCI_ARMADA100) += ehci-armada100.o utmi-armada100.o
obj-$(CONFIG_USB_EHCI_ATMEL) += ehci-atmel.o
ifdef CONFIG_MPC512X
obj-$(CONFIG_USB_EHCI_FSL) += ehci-mpc512x.o
else
obj-$(CONFIG_USB_EHCI_FSL) += ehci-fsl.o
endif
obj-$(CONFIG_USB_EHCI_FARADAY) += ehci-faraday.o
obj-$(CONFIG_USB_EHCI_EXYNOS) += ehci-exynos.o
obj-$(CONFIG_USB_EHCI_MXC) += ehci-mxc.o
obj-$(CONFIG_USB_EHCI_MXS) += ehci-mxs.o
obj-$(CONFIG_USB_EHCI_MX5) += ehci-mx5.o
obj-$(CONFIG_USB_EHCI_MX6) += ehci-mx6.o
obj-$(CONFIG_USB_EHCI_MX7) += ehci-mx6.o
obj-$(CONFIG_USB_EHCI_OMAP) += ehci-omap.o
obj-$(CONFIG_USB_EHCI_PPC4XX) += ehci-ppc4xx.o
obj-$(CONFIG_USB_EHCI_MARVELL) += ehci-marvell.o
obj-$(CONFIG_USB_EHCI_PCI) += ehci-pci.o
obj-$(CONFIG_USB_EHCI_SPEAR) += ehci-spear.o
obj-$(CONFIG_USB_EHCI_SUNXI) += ehci-sunxi.o
obj-$(CONFIG_USB_EHCI_TEGRA) += ehci-tegra.o
obj-$(CONFIG_USB_EHCI_UNIPHIER) += ehci-uniphier.o
obj-$(CONFIG_USB_EHCI_VCT) += ehci-vct.o
obj-$(CONFIG_USB_EHCI_VF) += ehci-vf.o
obj-$(CONFIG_USB_EHCI_RMOBILE) += ehci-rmobile.o
obj-$(CONFIG_USB_EHCI_ZYNQ) += ehci-zynq.o

# xhci
obj-$(CONFIG_USB_XHCI) += xhci.o xhci-mem.o xhci-ring.o
obj-$(CONFIG_USB_XHCI_DWC3) += xhci-dwc3.o
obj-$(CONFIG_USB_XHCI_KEYSTONE) += xhci-keystone.o
obj-$(CONFIG_USB_XHCI_EXYNOS) += xhci-exynos5.o
obj-$(CONFIG_USB_XHCI_FSL) += xhci-fsl.o
obj-$(CONFIG_USB_XHCI_OMAP) += xhci-omap.o
obj-$(CONFIG_USB_XHCI_PCI) += xhci-pci.o
obj-$(CONFIG_USB_XHCI_UNIPHIER) += xhci-uniphier.o

# designware
obj-$(CONFIG_USB_DWC2) += dwc2.o
