/*
 * (C) Copyright 2000
 * Wolfgang Denk, DENX Software Engineering, wd@denx.de.
 *
 * See file CREDITS for list of people who contributed to this
 * project.
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
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA 02111-1307 USA
 */

#ifndef	_IDE_H
#define _IDE_H

#define	IDE_BUS(dev)	(dev >> 1)

#define	ATA_CURR_BASE(dev)	(CONFIG_SYS_ATA_BASE_ADDR+ide_bus_offset[IDE_BUS(dev)])

#ifdef CONFIG_IDE_LED

/*
 * LED Port
 */
#define	LED_PORT	((uchar *)(PER8_BASE + 0x3000))
#define LED_IDE1	0x01
#define LED_IDE2	0x02
#define	DEVICE_LED(d)	((d & 2) | ((d & 2) == 0)) /* depends on bit positions! */

#endif /* CONFIG_IDE_LED */

#ifdef CONFIG_SYS_64BIT_LBA
typedef uint64_t lbaint_t;
#else
typedef ulong lbaint_t;
#endif

/*
 * Function Prototypes
 */

void ide_init(void);
ulong ide_read(int device, lbaint_t blknr, ulong blkcnt, void *buffer);
ulong ide_write(int device, lbaint_t blknr, ulong blkcnt, void *buffer);

#if defined(CONFIG_OF_IDE_FIXUP)
int ide_device_present(int dev);
#endif
#endif /* _IDE_H */
