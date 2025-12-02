/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright (c) 2024 Linaro Ltd.
 * Sam Protsenko <semen.protsenko@linaro.org>
 */

#ifndef __E850_96_FW_H
#define __E850_96_FW_H

#include <asm/types.h>

/* Image types for downloading over USB */
enum usb_dn_image {
	USB_DN_IMAGE_LDFW	= 1,	/* Loadable Firmware */
	USB_DN_IMAGE_SP		= 2,	/* Secure Payload (tzsw.img) */
};

int load_image_usb(enum usb_dn_image type, phys_addr_t addr, phys_size_t size);
int load_ldfw_from_blk(const char *ifname, int dev, int part, phys_addr_t addr);
int init_ldfw(phys_addr_t addr);

#endif /* __E850_96_FW_H */
