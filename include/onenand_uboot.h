/*
 *  Header file for OneNAND support for U-Boot
 *
 *  Adaptation from kernel to U-Boot
 *
 *  Copyright (C) 2005-2007 Samsung Electronics
 *  Kyungmin Park <kyungmin.park@samsung.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */

#ifndef __UBOOT_ONENAND_H
#define __UBOOT_ONENAND_H

struct kvec {
	void *iov_base;
	size_t iov_len;
};

typedef int spinlock_t;
typedef int wait_queue_head_t;

/* Functions */
extern void onenand_init(void);
extern int onenand_read(struct mtd_info *mtd, loff_t from, size_t len,
			size_t * retlen, u_char * buf);
extern int onenand_read_oob(struct mtd_info *mtd, loff_t from, size_t len,
			    size_t * retlen, u_char * buf);
extern int onenand_write(struct mtd_info *mtd, loff_t from, size_t len,
			 size_t * retlen, const u_char * buf);
extern int onenand_erase(struct mtd_info *mtd, struct erase_info *instr);

extern int onenand_unlock(struct mtd_info *mtd, loff_t ofs, size_t len);

extern void onenand_print_device_info(int device, int verbose);

#endif /* __UBOOT_ONENAND_H */
