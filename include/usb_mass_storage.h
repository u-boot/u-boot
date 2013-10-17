/*
 * Copyright (C) 2011 Samsung Electrnoics
 * Lukasz Majewski <l.majewski@samsung.com>
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#ifndef __USB_MASS_STORAGE_H__
#define __USB_MASS_STORAGE_H__

#define SECTOR_SIZE		0x200

#include <mmc.h>
#include <linux/usb/composite.h>

struct ums_device {
	struct mmc *mmc;
	int dev_num;
	int offset;
	int part_size;
};

struct ums_board_info {
	int (*read_sector)(struct ums_device *ums_dev,
			   ulong start, lbaint_t blkcnt, void *buf);
	int (*write_sector)(struct ums_device *ums_dev,
			    ulong start, lbaint_t blkcnt, const void *buf);
	void (*get_capacity)(struct ums_device *ums_dev,
			     long long int *capacity);
	const char *name;
	struct ums_device ums_dev;
};

extern void board_usb_init(void);

extern int fsg_init(struct ums_board_info *);
extern void fsg_cleanup(void);
extern struct ums_board_info *board_ums_init(unsigned int,
					     unsigned int, unsigned int);
extern int usb_gadget_handle_interrupts(void);
extern int fsg_main_thread(void *);

#ifdef CONFIG_USB_GADGET_MASS_STORAGE
int fsg_add(struct usb_configuration *c);
#else
int fsg_add(struct usb_configuration *c)
{
	return 0;
}
#endif
#endif /* __USB_MASS_STORAGE_H__ */
