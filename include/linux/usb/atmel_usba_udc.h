/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Platform data definitions for Atmel USBA gadget driver
 * [Original from Linux kernel: include/linux/usb/atmel_usba_udc.h]
 */
#ifndef __LINUX_USB_USBA_H__
#define __LINUX_USB_USBA_H__

struct usba_ep_data {
	char *name;
	int index;
	int fifo_size;
	int nr_banks;
	int can_dma;
	int can_isoc;
};

struct usba_platform_data {
	int			num_ep;
	struct usba_ep_data	*ep;
};

#if !CONFIG_IS_ENABLED(DM_USB_GADGET)
extern int usba_udc_probe(struct usba_platform_data *pdata);
#endif

#endif /* __LINUX_USB_USBA_H */
