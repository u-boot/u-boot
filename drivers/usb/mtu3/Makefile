# SPDX-License-Identifier: GPL-2.0

ccflags-$(CONFIG_$(PHASE_)USB_MTU3_DEBUG)	+= -DDEBUG

obj-$(CONFIG_$(PHASE_)USB_MTU3)	+= mtu3.o

mtu3-y	:= mtu3_plat.o

obj-$(CONFIG_$(PHASE_)USB_MTU3_GADGET) += mtu3_core.o mtu3_gadget_ep0.o
obj-$(CONFIG_$(PHASE_)USB_MTU3_GADGET) += mtu3_gadget.o mtu3_qmu.o
obj-$(CONFIG_$(PHASE_)USB_MTU3_HOST) += mtu3_host.o
