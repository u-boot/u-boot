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

#ifndef UMS_START_SECTOR
#define UMS_START_SECTOR	0
#endif

#ifndef UMS_NUM_SECTORS
#define UMS_NUM_SECTORS		0
#endif

/* Wait at maximum 60 seconds for cable connection */
#define UMS_CABLE_READY_TIMEOUT	60

struct ums {
	int (*read_sector)(struct ums *ums_dev,
			   ulong start, lbaint_t blkcnt, void *buf);
	int (*write_sector)(struct ums *ums_dev,
			    ulong start, lbaint_t blkcnt, const void *buf);
	unsigned int start_sector;
	unsigned int num_sectors;
	const char *name;
	struct mmc *mmc;
};

extern struct ums *ums;

int fsg_init(struct ums *);
void fsg_cleanup(void);
struct ums *ums_init(unsigned int);
int fsg_main_thread(void *);

#ifdef CONFIG_USB_GADGET_MASS_STORAGE
int fsg_add(struct usb_configuration *c);
#else
int fsg_add(struct usb_configuration *c)
{
	return 0;
}
#endif
#endif /* __USB_MASS_STORAGE_H__ */
