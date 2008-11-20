
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

#include <common.h>
#include <exports.h>
#include <asm/io.h>


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


void * memcpy(void * dest,const void *src,size_t count)
{
	char *tmp = (char *) dest, *s = (char *) src;

	while (count--)
		*tmp++ = *s++;

	return dest;
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

	printf("Resetting i82559 EEPROM @ 0x%08lx ... ", ioaddr);

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

static unsigned int hatoi(char *p, char **errp)
{
	unsigned int res = 0;

	while (1) {
		switch (*p) {
		case 'a':
		case 'b':
		case 'c':
		case 'd':
		case 'e':
		case 'f':
			res |= (*p - 'a' + 10);
			break;
		case 'A':
		case 'B':
		case 'C':
		case 'D':
		case 'E':
		case 'F':
			res |= (*p - 'A' + 10);
			break;
		case '0':
		case '1':
		case '2':
		case '3':
		case '4':
		case '5':
		case '6':
		case '7':
		case '8':
		case '9':
			res |= (*p - '0');
			break;
		default:
			if (errp) {
				*errp = p;
			}
		return res;
		}
		p++;
		if (*p == 0) {
			break;
		}
		res <<= 4;
	}

	if (errp) {
		*errp = NULL;
	}

	return res;
}

static unsigned char *gethwaddr(char *in, unsigned char *out)
{
	char tmp[3];
	int i;
	char *err;

	for (i=0;i<6;i++) {
		if (in[i*3+2] == 0 && i == 5) {
			out[i] = hatoi(&in[i*3], &err);
			if (err) {
				return NULL;
			}
		} else if (in[i*3+2] == ':' && i < 5) {
			tmp[0] = in[i*3];
			tmp[1] = in[i*3+1];
			tmp[2] = 0;
			out[i] = hatoi(tmp, &err);
			if (err) {
				return NULL;
			}
		} else {
			return NULL;
		}
	}

	return out;
}

static u32
read_config_dword(int bus, int dev, int func, int reg)
{
	u32 res;

	outl(0x80000000|(bus&0xff)<<16|(dev&0x1f)<<11|(func&7)<<8|(reg&0xfc),
	     0xcf8);
	res = inl(0xcfc);
	outl(0, 0xcf8);
	return res;
}

static u16
read_config_word(int bus, int dev, int func, int reg)
{
	u32 res;

	outl(0x80000000|(bus&0xff)<<16|(dev&0x1f)<<11|(func&7)<<8|(reg&0xfc),
	     0xcf8);
	res = inw(0xcfc + (reg & 2));
	outl(0, 0xcf8);
	return res;
}

static void
write_config_word(int bus, int dev, int func, int reg, u16 data)
{

	outl(0x80000000|(bus&0xff)<<16|(dev&0x1f)<<11|(func&7)<<8|(reg&0xfc),
	     0xcf8);
	outw(data, 0xcfc + (reg & 2));
	outl(0, 0xcf8);
}


int main (int argc, char *argv[])
{
	unsigned char *eth_addr;
	uchar buf[6];
	int instance;

	app_startup(argv);
	if (argc != 2) {
		printf ("call with base Ethernet address\n");
		return 1;
	}


	eth_addr = gethwaddr(argv[1], buf);
	if (NULL == eth_addr) {
		printf ("Can not parse ethernet address\n");
		return 1;
	}
	if (eth_addr[5] & 0x01) {
		printf("Base Ethernet address must be even\n");
	}


	for (instance = 0; instance < 2; instance ++)  {
		unsigned int io_addr;
		unsigned char mac[6];
		int bar1 = read_config_dword(0, 6+instance, 0, 0x14);
		if (! (bar1 & 1)) {
			printf("ETH%d is disabled %x\n", instance, bar1);
		} else {
			printf("ETH%d IO=0x%04x\n", instance, bar1 & ~3);
		}
		io_addr = (bar1 & (~3L));


		write_config_word(0, 6+instance, 0, 4,
				  read_config_word(0, 6+instance, 0, 4) | 1);
		printf("ETH%d CMD %04x\n", instance,
			   read_config_word(0, 6+instance, 0, 4));

		memcpy(mac, eth_addr, 6);
		mac[5] += instance;

		printf("got io=%04x, ha=%02x:%02x:%02x:%02x:%02x:%02x\n",
			   io_addr, mac[0], mac[1], mac[2],
			   mac[3], mac[4], mac[5]);
		reset_eeprom(io_addr, mac);
	}
	return 0;
}
