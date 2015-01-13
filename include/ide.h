/*
 * (C) Copyright 2000
 * Wolfgang Denk, DENX Software Engineering, wd@denx.de.
 *
 * SPDX-License-Identifier:	GPL-2.0+
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

void ide_led(uchar led, uchar status);
#endif /* CONFIG_IDE_LED */

#ifdef CONFIG_SYS_64BIT_LBA
typedef uint64_t lbaint_t;
#define LBAF "%llx"
#define LBAFU "%llu"
#else
typedef ulong lbaint_t;
#define LBAF "%lx"
#define LBAFU "%lu"
#endif

/*
 * Function Prototypes
 */

void ide_init(void);
ulong ide_read(int device, lbaint_t blknr, lbaint_t blkcnt, void *buffer);
ulong ide_write(int device, lbaint_t blknr, lbaint_t blkcnt,
		const void *buffer);

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
void ide_write_data(int dev, const ulong *sect_buf, int words);
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

/**
 * board_start_ide() - Start up the board IDE interfac
 *
 * @return 0 if ok
 */
int board_start_ide(void);

#endif /* _IDE_H */
