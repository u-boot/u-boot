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

void ide_init(void);
struct blk_desc;
struct udevice;
ulong ide_read(struct udevice *dev, lbaint_t blknr, lbaint_t blkcnt,
	       void *buffer);
ulong ide_write(struct udevice *dev, lbaint_t blknr, lbaint_t blkcnt,
		const void *buffer);

#if defined(CONFIG_OF_IDE_FIXUP)
int ide_device_present(int dev);
#endif

/*
 * I/O function overrides
 */
unsigned char ide_inb(int dev, int port);
void ide_outb(int dev, int port, unsigned char val);
void ide_input_swap_data(int dev, ulong *sect_buf, int words);
void ide_input_data(int dev, ulong *sect_buf, int words);
void ide_output_data(int dev, const ulong *sect_buf, int words);
void ide_input_data_shorts(int dev, ushort *sect_buf, int shorts);
void ide_output_data_shorts(int dev, ushort *sect_buf, int shorts);

void ide_led(uchar led, uchar status);

#endif /* _IDE_H */
