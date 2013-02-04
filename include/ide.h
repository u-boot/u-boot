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

#define IDE_BUS(dev)	(dev / (CONFIG_SYS_IDE_MAXDEVICE / CONFIG_SYS_IDE_MAXBUS))

#define	ATA_CURR_BASE(dev)	(CONFIG_SYS_ATA_BASE_ADDR+ide_bus_offset[IDE_BUS(dev)])
extern ulong ide_bus_offset[];

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
#define LBAF "%llx"
#else
typedef ulong lbaint_t;
#define LBAF "%lx"
#endif

/*
 * Function Prototypes
 */

void ide_init(void);
ulong ide_read(int device, ulong blknr, lbaint_t blkcnt, void *buffer);
ulong ide_write(int device, ulong blknr, lbaint_t blkcnt, const void *buffer);

#ifdef CONFIG_IDE_PREINIT
int ide_preinit(void);
#endif

#ifdef CONFIG_IDE_INIT_POSTRESET
int ide_init_postreset(void);
#endif

#if defined(CONFIG_OF_IDE_FIXUP)
int ide_device_present(int dev);
#endif

#if defined(CONFIG_IDE_AHB)
unsigned char ide_read_register(int dev, unsigned int port);
void ide_write_register(int dev, unsigned int port, unsigned char val);
void ide_read_data(int dev, ulong *sect_buf, int words);
void ide_write_data(int dev, ulong *sect_buf, int words);
#endif

/*
 * I/O function overrides
 */
void ide_input_swap_data(int dev, ulong *sect_buf, int words);
void ide_input_data(int dev, ulong *sect_buf, int words);
void ide_output_data(int dev, const ulong *sect_buf, int words);
void ide_input_data_shorts(int dev, ushort *sect_buf, int shorts);
void ide_output_data_shorts(int dev, ushort *sect_buf, int shorts);

#endif /* _IDE_H */
