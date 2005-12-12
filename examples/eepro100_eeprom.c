/*
 * Copyright 1998-2001 by Donald Becker.
 * This software may be used and distributed according to the terms of
 * the GNU General Public License (GPL), incorporated herein by reference.
 * Contact the author for use under other terms.
 *
 * This program must be compiled with "-O"!
 * See the bottom of this file for the suggested compile-command.
 *
 * The author may be reached as becker@scyld.com, or C/O
 *  Scyld Computing Corporation
 *  410 Severn Ave., Suite 210
 *  Annapolis MD 21403
 *
 * Common-sense licensing statement: Using any portion of this program in
 * your own program means that you must give credit to the original author
 * and release the resulting code under the GPL.
 */

#define _PPC_STRING_H_		/* avoid unnecessary str/mem functions */
#define _LINUX_STRING_H_	/* avoid unnecessary str/mem functions */

#include <common.h>
#include <exports.h>

static int reset_eeprom(unsigned long ioaddr, unsigned char *hwaddr);

int eepro100_eeprom(int argc, char *argv[])
{
	int ret = 0;

	unsigned char hwaddr1[6] = { 0x00, 0x00, 0x02, 0x03, 0x04, 0x05 };
	unsigned char hwaddr2[6] = { 0x00, 0x00, 0x02, 0x03, 0x04, 0x06 };

	app_startup(argv);

#if defined(CONFIG_OXC)
	ret |= reset_eeprom(0x80000000, hwaddr1);
	ret |= reset_eeprom(0x81000000, hwaddr2);
#endif

	return ret;
}

/* Default EEPROM for i82559 */
static unsigned short default_eeprom[64] = {
	0x0100, 0x0302, 0x0504, 0xffff, 0xffff, 0xffff, 0xffff, 0xffff,
	0xffff, 0xffff, 0x40c0, 0x0000, 0x0000, 0xffff, 0xffff, 0xffff,
	0xffff, 0xffff, 0xffff, 0xffff, 0xffff, 0xffff, 0xffff, 0xffff,
	0xffff, 0xffff, 0xffff, 0xffff, 0xffff, 0xffff, 0xffff, 0xffff,
	0xffff, 0xffff, 0xffff, 0xffff, 0xffff, 0xffff, 0xffff, 0xffff,
	0xffff, 0xffff, 0xffff, 0xffff, 0xffff, 0xffff, 0xffff, 0xffff,
	0xffff, 0xffff, 0xffff, 0xffff, 0xffff, 0xffff, 0xffff, 0xffff,
	0xffff, 0xffff, 0xffff, 0xffff, 0xffff, 0xffff, 0xffff, 0xffff
};

static unsigned short eeprom[256];

static int eeprom_size = 64;
static int eeprom_addr_size = 6;

static int debug = 0;

static inline unsigned short swap16(unsigned short x)
{
	return (((x & 0xff) << 8) | ((x & 0xff00) >> 8));
}

static inline void outw(short data, long addr)
{
	*(volatile short *)(addr) = swap16(data);
}

static inline short inw(long addr)
{
	return swap16(*(volatile short *)(addr));
}

static inline void *memcpy(void *dst, const void *src, unsigned int len)
{
	char *ret = dst;
	while (len-- > 0) {
		*ret++ = *((char *)src);
		src++;
	}
	return (void *)ret;
}

/* The EEPROM commands include the alway-set leading bit. */
#define EE_WRITE_CMD	(5)
#define EE_READ_CMD		(6)
#define EE_ERASE_CMD	(7)

/* Serial EEPROM section. */
#define EE_SHIFT_CLK	0x01	/* EEPROM shift clock. */
#define EE_CS			0x02	/* EEPROM chip select. */
#define EE_DATA_WRITE	0x04	/* EEPROM chip data in. */
#define EE_DATA_READ	0x08	/* EEPROM chip data out. */
#define EE_ENB			(0x4800 | EE_CS)
#define EE_WRITE_0		0x4802
#define EE_WRITE_1		0x4806
#define EE_OFFSET		14

/* Delay between EEPROM clock transitions. */
#define eeprom_delay(ee_addr)	inw(ee_addr)

/* Wait for the EEPROM to finish the previous operation. */
static int eeprom_busy_poll(long ee_ioaddr)
{
	int i;
	outw(EE_ENB, ee_ioaddr);
	for (i = 0; i < 10000; i++)			/* Typical 2000 ticks */
		if (inw(ee_ioaddr) & EE_DATA_READ)
			break;
	return i;
}

/* This executes a generic EEPROM command, typically a write or write enable.
   It returns the data output from the EEPROM, and thus may also be used for
   reads. */
static int do_eeprom_cmd(long ioaddr, int cmd, int cmd_len)
{
	unsigned retval = 0;
	long ee_addr = ioaddr + EE_OFFSET;

	if (debug > 1)
		printf(" EEPROM op 0x%x: ", cmd);

	outw(EE_ENB | EE_SHIFT_CLK, ee_addr);

	/* Shift the command bits out. */
	do {
		short dataval = (cmd & (1 << cmd_len)) ? EE_WRITE_1 : EE_WRITE_0;
		outw(dataval, ee_addr);
		eeprom_delay(ee_addr);
		if (debug > 2)
			printf("%X", inw(ee_addr) & 15);
		outw(dataval | EE_SHIFT_CLK, ee_addr);
		eeprom_delay(ee_addr);
		retval = (retval << 1) | ((inw(ee_addr) & EE_DATA_READ) ? 1 : 0);
	} while (--cmd_len >= 0);
#if 0
	outw(EE_ENB, ee_addr);
#endif
	/* Terminate the EEPROM access. */
	outw(EE_ENB & ~EE_CS, ee_addr);
	if (debug > 1)
		printf(" EEPROM result is 0x%5.5x.\n", retval);
	return retval;
}

static int read_eeprom(long ioaddr, int location, int addr_len)
{
	return do_eeprom_cmd(ioaddr, ((EE_READ_CMD << addr_len) | location)
		<< 16 , 3 + addr_len + 16) & 0xffff;
}

static void write_eeprom(long ioaddr, int index, int value, int addr_len)
{
	long ee_ioaddr = ioaddr + EE_OFFSET;
	int i;

	/* Poll for previous op finished. */
	eeprom_busy_poll(ee_ioaddr);			/* Typical 0 ticks */
	/* Enable programming modes. */
	do_eeprom_cmd(ioaddr, (0x4f << (addr_len-4)), 3 + addr_len);
	/* Do the actual write. */
	do_eeprom_cmd(ioaddr,
				  (((EE_WRITE_CMD<<addr_len) | index)<<16) | (value & 0xffff),
				  3 + addr_len + 16);
	/* Poll for write finished. */
	i = eeprom_busy_poll(ee_ioaddr);			/* Typical 2000 ticks */
	if (debug)
		printf(" Write finished after %d ticks.\n", i);
	/* Disable programming. This command is not instantaneous, so we check
	   for busy before the next op. */
	do_eeprom_cmd(ioaddr, (0x40 << (addr_len-4)), 3 + addr_len);
	eeprom_busy_poll(ee_ioaddr);
}

static int reset_eeprom(unsigned long ioaddr, unsigned char *hwaddr)
{
	unsigned short checksum = 0;
	int size_test;
	int i;

	printf("Resetting i82559 EEPROM @ 0x%08lX ... ", ioaddr);

	size_test = do_eeprom_cmd(ioaddr, (EE_READ_CMD << 8) << 16, 27);
	eeprom_addr_size = (size_test & 0xffe0000) == 0xffe0000 ? 8 : 6;
	eeprom_size = 1 << eeprom_addr_size;

	memcpy(eeprom, default_eeprom, sizeof default_eeprom);

	for (i = 0; i < 3; i++)
		eeprom[i] = (hwaddr[i*2+1]<<8) + hwaddr[i*2];

	/* Recalculate the checksum. */
	for (i = 0; i < eeprom_size - 1; i++)
		checksum += eeprom[i];
	eeprom[i] = 0xBABA - checksum;

	for (i = 0; i < eeprom_size; i++)
		write_eeprom(ioaddr, i, eeprom[i], eeprom_addr_size);

	for (i = 0; i < eeprom_size; i++)
		if (read_eeprom(ioaddr, i, eeprom_addr_size) != eeprom[i]) {
			printf("failed\n");
			return 1;
		}

	printf("done\n");
	return 0;
}
