/*
 * Copyright (C) 2011 Samsung Electrnoics
 * Lukasz Majewski <l.majewski@samsung.com>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * aloong with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA 02111-1307 USA
 */

#ifndef __USB_MASS_STORAGE_H__
#define __USB_MASS_STORAGE_H__

#define SECTOR_SIZE		0x200

#include <mmc.h>

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

#endif /* __USB_MASS_STORAGE_H__ */
