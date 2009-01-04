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

LIB	:= $(obj)libusb.a

# core
COBJS-y += usbdcore.o
COBJS-$(CONFIG_USB_OHCI_NEW) += usb_ohci.o
COBJS-$(CONFIG_USB_EHCI) += usb_ehci_core.o

# host
COBJS-$(CONFIG_USB_ISP116X_HCD) += isp116x-hcd.o
COBJS-$(CONFIG_USB_R8A66597_HCD) += r8a66597-hcd.o
COBJS-$(CONFIG_USB_SL811HS) += sl811_usb.o
COBJS-$(CONFIG_USB_EHCI_FSL) += usb_ehci_fsl.o
COBJS-$(CONFIG_USB_EHCI_PCI) += usb_ehci_pci.o
COBJS-$(CONFIG_USB_EHCI_IXP4XX) += usb_ehci_ixp.o
COBJS-$(CONFIG_MUSB_HCD) += musb_hcd.o musb_core.o
COBJS-$(CONFIG_USB_DAVINCI) += davinci_usb.o

# device
ifdef CONFIG_USB_DEVICE
COBJS-y += usbdcore_ep0.o
COBJS-$(CONFIG_OMAP1510) += usbdcore_omap1510.o
COBJS-$(CONFIG_OMAP1610) += usbdcore_omap1510.o
COBJS-$(CONFIG_MPC885_FAMILY) += usbdcore_mpc8xx.o
endif

COBJS	:= $(COBJS-y)
SRCS	:= $(COBJS:.o=.c)
OBJS	:= $(addprefix $(obj),$(COBJS))

all:	$(LIB)

$(LIB):	$(obj).depend $(OBJS)
	$(AR) $(ARFLAGS) $@ $(OBJS)

#########################################################################

# defines $(obj).depend target
include $(SRCTREE)/rules.mk

sinclude $(obj).depend

#########################################################################
