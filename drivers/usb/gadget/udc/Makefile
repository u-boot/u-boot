# SPDX-License-Identifier: GPL-2.0+
#
# USB peripheral controller drivers

ifndef CONFIG_$(XPL_)DM_USB_GADGET
obj-$(CONFIG_USB_DWC3_GADGET)	+= udc-core.o
endif

obj-$(CONFIG_$(XPL_)DM_USB_GADGET)	+= udc-core.o
obj-y += udc-uclass.o
