/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * (C) Copyright 2000
 * Wolfgang Denk, DENX Software Engineering, wd@denx.de.
 */

#ifndef	_IDE_H
#define _IDE_H

#include <blk.h>

#define IDE_BUS(dev)	(dev / (CONFIG_SYS_IDE_MAXDEVICE / CONFIG_SYS_IDE_MAXBUS))

/*
 * Function Prototypes
 */

struct blk_desc;
struct udevice;
ulong ide_read(struct udevice *dev, lbaint_t blknr, lbaint_t blkcnt,
	       void *buffer);
ulong ide_write(struct udevice *dev, lbaint_t blknr, lbaint_t blkcnt,
		const void *buffer);

#endif /* _IDE_H */
