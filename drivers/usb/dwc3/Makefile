# SPDX-License-Identifier: GPL-2.0
obj-$(CONFIG_USB_DWC3)			+= dwc3.o

dwc3-y					:= core.o

ifneq ($(filter y,$(CONFIG_USB_DWC3_HOST) $(CONFIG_USB_DWC3_DUAL_ROLE)),)
	dwc3-y				+= host.o
endif

ifneq ($(filter y,$(CONFIG_USB_DWC3_GADGET) $(CONFIG_USB_DWC3_DUAL_ROLE)),)
	dwc3-y				+= gadget.o ep0.o
endif

ifneq ($(CONFIG_USB_DWC3_DUAL_ROLE),)
	dwc3-y				+= drd.o
endif

ifneq ($(CONFIG_USB_DWC3_ULPI),)
	dwc3-y				+= ulpi.o
endif

ifneq ($(CONFIG_DEBUG_FS),)
	dwc3-y				+= debugfs.o
endif

##
# Platform-specific glue layers go here
#
# NOTICE: Make sure your glue layer doesn't depend on anything
# which is arch-specific and that it compiles on all situations.
#
# We want to keep this requirement in order to be able to compile
# the entire driver (with all its glue layers) on several architectures
# and make sure it compiles fine. This will also help with allmodconfig
# and allyesconfig builds.
##

obj-$(CONFIG_$(PHASE_)USB_DWC3_AM62)		+= dwc3-am62.o
obj-$(CONFIG_USB_DWC3_OMAP)		+= dwc3-omap.o
obj-$(CONFIG_USB_DWC3_EXYNOS)		+= dwc3-exynos.o
obj-$(CONFIG_$(PHASE_)USB_DWC3_GENERIC)	+= dwc3-generic.o
obj-$(CONFIG_USB_DWC3_STI)		+= dwc3-generic-sti.o
obj-$(CONFIG_USB_DWC3_PCI)		+= dwc3-pci.o
obj-$(CONFIG_USB_DWC3_HAPS)		+= dwc3-haps.o
obj-$(CONFIG_USB_DWC3_IMX8MP)		+= dwc3-imx8mp.o
obj-$(CONFIG_USB_DWC3_KEYSTONE)		+= dwc3-keystone.o
obj-$(CONFIG_USB_DWC3_LAYERSCAPE)	+= dwc3-layerscape.o
obj-$(CONFIG_USB_DWC3_MESON_G12A)	+= dwc3-meson-g12a.o
obj-$(CONFIG_USB_DWC3_MESON_GXL)	+= dwc3-meson-gxl.o
obj-$(CONFIG_USB_DWC3_OCTEON)		+= dwc3-octeon.o
obj-$(CONFIG_USB_DWC3_OF_SIMPLE)	+= dwc3-of-simple.o
obj-$(CONFIG_USB_DWC3_QCOM)		+= dwc3-qcom.o
obj-$(CONFIG_USB_DWC3_QCOM)		+= dwc3-qcom-legacy.o
obj-$(CONFIG_USB_DWC3_RTK)		+= dwc3-rtk.o
obj-$(CONFIG_USB_DWC3_ST)		+= dwc3-st.o
obj-$(CONFIG_USB_DWC3_UNIPHIER)		+= dwc3-uniphier.o
obj-$(CONFIG_USB_DWC3_XILINX)		+= dwc3-xilinx.o
obj-$(CONFIG_USB_DWC3_PHY_OMAP)		+= ti_usb_phy.o
obj-$(CONFIG_$(PHASE_)USB_DWC3_GENERIC)		+= dwc3-generic.o
