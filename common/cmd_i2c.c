/*
 * (C) Copyright 2001
 * Gerald Van Baren, Custom IDEAS, vanbaren@cideas.com.
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

/*
 * I2C Functions similar to the standard memory functions.
 *
 * There are several parameters in many of the commands that bear further
 * explanations:
 *
 * {i2c_chip} is the I2C chip address (the first byte sent on the bus).
 *   Each I2C chip on the bus has a unique address.  On the I2C data bus,
 *   the address is the upper seven bits and the LSB is the "read/write"
 *   bit.  Note that the {i2c_chip} address specified on the command
 *   line is not shifted up: e.g. a typical EEPROM memory chip may have
 *   an I2C address of 0x50, but the data put on the bus will be 0xA0
 *   for write and 0xA1 for read.  This "non shifted" address notation
 *   matches at least half of the data sheets :-/.
 *
 * {addr} is the address (or offset) within the chip.  Small memory
 *   chips have 8 bit addresses.  Large memory chips have 16 bit
 *   addresses.  Other memory chips have 9, 10, or 11 bit addresses.
 *   Many non-memory chips have multiple registers and {addr} is used
 *   as the register index.  Some non-memory chips have only one register
 *   and therefore don't need any {addr} parameter.
 *
 *   The default {addr} parameter is one byte (.1) which works well for
 *   memories and registers with 8 bits of address space.
 *
 *   You can specify the length of the {addr} field with the optional .0,
 *   .1, or .2 modifier (similar to the .b, .w, .l modifier).  If you are
 *   manipulating a single register device which doesn't use an address
 *   field, use "0.0" for the address and the ".0" length field will
 *   suppress the address in the I2C data stream.  This also works for
 *   successive reads using the I2C auto-incrementing memory pointer.
 *
 *   If you are manipulating a large memory with 2-byte addresses, use
 *   the .2 address modifier, e.g. 210.2 addresses location 528 (decimal).
 *
 *   Then there are the unfortunate memory chips that spill the most
 *   significant 1, 2, or 3 bits of address into the chip address byte.
 *   This effectively makes one chip (logically) look like 2, 4, or
 *   8 chips.  This is handled (awkwardly) by #defining
 *   CONFIG_SYS_I2C_EEPROM_ADDR_OVERFLOW and using the .1 modifier on the
 *   {addr} field (since .1 is the default, it doesn't actually have to
 *   be specified).  Examples: given a memory chip at I2C chip address
 *   0x50, the following would happen...
 *     i2c md 50 0 10   display 16 bytes starting at 0x000
 *                      On the bus: <S> A0 00 <E> <S> A1 <rd> ... <rd>
 *     i2c md 50 100 10 display 16 bytes starting at 0x100
 *                      On the bus: <S> A2 00 <E> <S> A3 <rd> ... <rd>
 *     i2c md 50 210 10 display 16 bytes starting at 0x210
 *                      On the bus: <S> A4 10 <E> <S> A5 <rd> ... <rd>
 *   This is awfully ugly.  It would be nice if someone would think up
 *   a better way of handling this.
 *
 * Adapted from cmd_mem.c which is copyright Wolfgang Denk (wd@denx.de).
 */

#include <common.h>
#include <command.h>
#include <environment.h>
#include <i2c.h>
#include <malloc.h>
#include <asm/byteorder.h>

/* Display values from last command.
 * Memory modify remembered values are different from display memory.
 */
static uchar	i2c_dp_last_chip;
static uint	i2c_dp_last_addr;
static uint	i2c_dp_last_alen;
static uint	i2c_dp_last_length = 0x10;

static uchar	i2c_mm_last_chip;
static uint	i2c_mm_last_addr;
static uint	i2c_mm_last_alen;

/* If only one I2C bus is present, the list of devices to ignore when
 * the probe command is issued is represented by a 1D array of addresses.
 * When multiple buses are present, the list is an array of bus-address
 * pairs.  The following macros take care of this */

#if defined(CONFIG_SYS_I2C_NOPROBES)
#if defined(CONFIG_I2C_MULTI_BUS)
static struct
{
	uchar	bus;
	uchar	addr;
} i2c_no_probes[] = CONFIG_SYS_I2C_NOPROBES;
#define GET_BUS_NUM	i2c_get_bus_num()
#define COMPARE_BUS(b,i)	(i2c_no_probes[(i)].bus == (b))
#define COMPARE_ADDR(a,i)	(i2c_no_probes[(i)].addr == (a))
#define NO_PROBE_ADDR(i)	i2c_no_probes[(i)].addr
#else		/* single bus */
static uchar i2c_no_probes[] = CONFIG_SYS_I2C_NOPROBES;
#define GET_BUS_NUM	0
#define COMPARE_BUS(b,i)	((b) == 0)	/* Make compiler happy */
#define COMPARE_ADDR(a,i)	(i2c_no_probes[(i)] == (a))
#define NO_PROBE_ADDR(i)	i2c_no_probes[(i)]
#endif	/* CONFIG_MULTI_BUS */

#define NUM_ELEMENTS_NOPROBE (sizeof(i2c_no_probes)/sizeof(i2c_no_probes[0]))
#endif

#if defined(CONFIG_I2C_MUX)
static I2C_MUX_DEVICE	*i2c_mux_devices = NULL;
static	int	i2c_mux_busid = CONFIG_SYS_MAX_I2C_BUS;

DECLARE_GLOBAL_DATA_PTR;

#endif

#define DISP_LINE_LEN	16

/* implement possible board specific board init */
void __def_i2c_init_board(void)
{
	return;
}
void i2c_init_board(void)
	__attribute__((weak, alias("__def_i2c_init_board")));

/* TODO: Implement architecture-specific get/set functions */
unsigned int __def_i2c_get_bus_speed(void)
{
	return CONFIG_SYS_I2C_SPEED;
}
unsigned int i2c_get_bus_speed(void)
	__attribute__((weak, alias("__def_i2c_get_bus_speed")));

int __def_i2c_set_bus_speed(unsigned int speed)
{
	if (speed != CONFIG_SYS_I2C_SPEED)
		return -1;

	return 0;
}
int i2c_set_bus_speed(unsigned int)
	__attribute__((weak, alias("__def_i2c_set_bus_speed")));

/*
 * get_alen: small parser helper function to get address length
 * returns the address length
 */
static uint get_alen(char *arg)
{
	int	j;
	int	alen;

	alen = 1;
	for (j = 0; j < 8; j++) {
		if (arg[j] == '.') {
			alen = arg[j+1] - '0';
			break;
		} else if (arg[j] == '\0')
			break;
	}
	return alen;
}

/*
 * Syntax:
 *	i2c read {i2c_chip} {devaddr}{.0, .1, .2} {len} {memaddr}
 */

static int do_i2c_read ( cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[])
{
	u_char	chip;
	uint	devaddr, alen, length;
	u_char  *memaddr;

	if (argc != 5)
		return cmd_usage(cmdtp);

	/*
	 * I2C chip address
	 */
	chip = simple_strtoul(argv[1], NULL, 16);

	/*
	 * I2C data address within the chip.  This can be 1 or
	 * 2 bytes long.  Some day it might be 3 bytes long :-).
	 */
	devaddr = simple_strtoul(argv[2], NULL, 16);
	alen = get_alen(argv[2]);
	if (alen > 3)
		return cmd_usage(cmdtp);

	/*
	 * Length is the number of objects, not number of bytes.
	 */
	length = simple_strtoul(argv[3], NULL, 16);

	/*
	 * memaddr is the address where to store things in memory
	 */
	memaddr = (u_char *)simple_strtoul(argv[4], NULL, 16);

	if (i2c_read(chip, devaddr, alen, memaddr, length) != 0) {
		puts ("Error reading the chip.\n");
		return 1;
	}
	return 0;
}

/*
 * Syntax:
 *	i2c md {i2c_chip} {addr}{.0, .1, .2} {len}
 */
static int do_i2c_md ( cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[])
{
	u_char	chip;
	uint	addr, alen, length;
	int	j, nbytes, linebytes;

	/* We use the last specified parameters, unless new ones are
	 * entered.
	 */
	chip   = i2c_dp_last_chip;
	addr   = i2c_dp_last_addr;
	alen   = i2c_dp_last_alen;
	length = i2c_dp_last_length;

	if (argc < 3)
		return cmd_usage(cmdtp);

	if ((flag & CMD_FLAG_REPEAT) == 0) {
		/*
		 * New command specified.
		 */

		/*
		 * I2C chip address
		 */
		chip = simple_strtoul(argv[1], NULL, 16);

		/*
		 * I2C data address within the chip.  This can be 1 or
		 * 2 bytes long.  Some day it might be 3 bytes long :-).
		 */
		addr = simple_strtoul(argv[2], NULL, 16);
		alen = get_alen(argv[2]);
		if (alen > 3)
			return cmd_usage(cmdtp);

		/*
		 * If another parameter, it is the length to display.
		 * Length is the number of objects, not number of bytes.
		 */
		if (argc > 3)
			length = simple_strtoul(argv[3], NULL, 16);
	}

	/*
	 * Print the lines.
	 *
	 * We buffer all read data, so we can make sure data is read only
	 * once.
	 */
	nbytes = length;
	do {
		unsigned char	linebuf[DISP_LINE_LEN];
		unsigned char	*cp;

		linebytes = (nbytes > DISP_LINE_LEN) ? DISP_LINE_LEN : nbytes;

		if (i2c_read(chip, addr, alen, linebuf, linebytes) != 0)
			puts ("Error reading the chip.\n");
		else {
			printf("%04x:", addr);
			cp = linebuf;
			for (j=0; j<linebytes; j++) {
				printf(" %02x", *cp++);
				addr++;
			}
			puts ("    ");
			cp = linebuf;
			for (j=0; j<linebytes; j++) {
				if ((*cp < 0x20) || (*cp > 0x7e))
					puts (".");
				else
					printf("%c", *cp);
				cp++;
			}
			putc ('\n');
		}
		nbytes -= linebytes;
	} while (nbytes > 0);

	i2c_dp_last_chip   = chip;
	i2c_dp_last_addr   = addr;
	i2c_dp_last_alen   = alen;
	i2c_dp_last_length = length;

	return 0;
}


/* Write (fill) memory
 *
 * Syntax:
 *	i2c mw {i2c_chip} {addr}{.0, .1, .2} {data} [{count}]
 */
static int do_i2c_mw ( cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[])
{
	uchar	chip;
	ulong	addr;
	uint	alen;
	uchar	byte;
	int	count;

	if ((argc < 4) || (argc > 5))
		return cmd_usage(cmdtp);

	/*
	 * Chip is always specified.
	 */
	chip = simple_strtoul(argv[1], NULL, 16);

	/*
	 * Address is always specified.
	 */
	addr = simple_strtoul(argv[2], NULL, 16);
	alen = get_alen(argv[2]);
	if (alen > 3)
		return cmd_usage(cmdtp);

	/*
	 * Value to write is always specified.
	 */
	byte = simple_strtoul(argv[3], NULL, 16);

	/*
	 * Optional count
	 */
	if (argc == 5)
		count = simple_strtoul(argv[4], NULL, 16);
	else
		count = 1;

	while (count-- > 0) {
		if (i2c_write(chip, addr++, alen, &byte, 1) != 0)
			puts ("Error writing the chip.\n");
		/*
		 * Wait for the write to complete.  The write can take
		 * up to 10mSec (we allow a little more time).
		 */
/*
 * No write delay with FRAM devices.
 */
#if !defined(CONFIG_SYS_I2C_FRAM)
		udelay(11000);
#endif
	}

	return (0);
}

/* Calculate a CRC on memory
 *
 * Syntax:
 *	i2c crc32 {i2c_chip} {addr}{.0, .1, .2} {count}
 */
static int do_i2c_crc (cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[])
{
	uchar	chip;
	ulong	addr;
	uint	alen;
	int	count;
	uchar	byte;
	ulong	crc;
	ulong	err;

	if (argc < 4)
		return cmd_usage(cmdtp);

	/*
	 * Chip is always specified.
	 */
	chip = simple_strtoul(argv[1], NULL, 16);

	/*
	 * Address is always specified.
	 */
	addr = simple_strtoul(argv[2], NULL, 16);
	alen = get_alen(argv[2]);
	if (alen > 3)
		return cmd_usage(cmdtp);

	/*
	 * Count is always specified
	 */
	count = simple_strtoul(argv[3], NULL, 16);

	printf ("CRC32 for %08lx ... %08lx ==> ", addr, addr + count - 1);
	/*
	 * CRC a byte at a time.  This is going to be slooow, but hey, the
	 * memories are small and slow too so hopefully nobody notices.
	 */
	crc = 0;
	err = 0;
	while (count-- > 0) {
		if (i2c_read(chip, addr, alen, &byte, 1) != 0)
			err++;
		crc = crc32 (crc, &byte, 1);
		addr++;
	}
	if (err > 0)
		puts ("Error reading the chip,\n");
	else
		printf ("%08lx\n", crc);

	return 0;
}

/* Modify memory.
 *
 * Syntax:
 *	i2c mm{.b, .w, .l} {i2c_chip} {addr}{.0, .1, .2}
 *	i2c nm{.b, .w, .l} {i2c_chip} {addr}{.0, .1, .2}
 */

static int
mod_i2c_mem(cmd_tbl_t *cmdtp, int incrflag, int flag, int argc, char * const argv[])
{
	uchar	chip;
	ulong	addr;
	uint	alen;
	ulong	data;
	int	size = 1;
	int	nbytes;
	extern char console_buffer[];

	if (argc != 3)
		return cmd_usage(cmdtp);

#ifdef CONFIG_BOOT_RETRY_TIME
	reset_cmd_timeout();	/* got a good command to get here */
#endif
	/*
	 * We use the last specified parameters, unless new ones are
	 * entered.
	 */
	chip = i2c_mm_last_chip;
	addr = i2c_mm_last_addr;
	alen = i2c_mm_last_alen;

	if ((flag & CMD_FLAG_REPEAT) == 0) {
		/*
		 * New command specified.  Check for a size specification.
		 * Defaults to byte if no or incorrect specification.
		 */
		size = cmd_get_data_size(argv[0], 1);

		/*
		 * Chip is always specified.
		 */
		chip = simple_strtoul(argv[1], NULL, 16);

		/*
		 * Address is always specified.
		 */
		addr = simple_strtoul(argv[2], NULL, 16);
		alen = get_alen(argv[2]);
		if (alen > 3)
			return cmd_usage(cmdtp);
	}

	/*
	 * Print the address, followed by value.  Then accept input for
	 * the next value.  A non-converted value exits.
	 */
	do {
		printf("%08lx:", addr);
		if (i2c_read(chip, addr, alen, (uchar *)&data, size) != 0)
			puts ("\nError reading the chip,\n");
		else {
			data = cpu_to_be32(data);
			if (size == 1)
				printf(" %02lx", (data >> 24) & 0x000000FF);
			else if (size == 2)
				printf(" %04lx", (data >> 16) & 0x0000FFFF);
			else
				printf(" %08lx", data);
		}

		nbytes = readline (" ? ");
		if (nbytes == 0) {
			/*
			 * <CR> pressed as only input, don't modify current
			 * location and move to next.
			 */
			if (incrflag)
				addr += size;
			nbytes = size;
#ifdef CONFIG_BOOT_RETRY_TIME
			reset_cmd_timeout(); /* good enough to not time out */
#endif
		}
#ifdef CONFIG_BOOT_RETRY_TIME
		else if (nbytes == -2)
			break;	/* timed out, exit the command	*/
#endif
		else {
			char *endp;

			data = simple_strtoul(console_buffer, &endp, 16);
			if (size == 1)
				data = data << 24;
			else if (size == 2)
				data = data << 16;
			data = be32_to_cpu(data);
			nbytes = endp - console_buffer;
			if (nbytes) {
#ifdef CONFIG_BOOT_RETRY_TIME
				/*
				 * good enough to not time out
				 */
				reset_cmd_timeout();
#endif
				if (i2c_write(chip, addr, alen, (uchar *)&data, size) != 0)
					puts ("Error writing the chip.\n");
#ifdef CONFIG_SYS_EEPROM_PAGE_WRITE_DELAY_MS
				udelay(CONFIG_SYS_EEPROM_PAGE_WRITE_DELAY_MS * 1000);
#endif
				if (incrflag)
					addr += size;
			}
		}
	} while (nbytes);

	i2c_mm_last_chip = chip;
	i2c_mm_last_addr = addr;
	i2c_mm_last_alen = alen;

	return 0;
}

/*
 * Syntax:
 *	i2c probe {addr}{.0, .1, .2}
 */
static int do_i2c_probe (cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[])
{
	int j;
#if defined(CONFIG_SYS_I2C_NOPROBES)
	int k, skip;
	uchar bus = GET_BUS_NUM;
#endif	/* NOPROBES */

	puts ("Valid chip addresses:");
	for (j = 0; j < 128; j++) {
#if defined(CONFIG_SYS_I2C_NOPROBES)
		skip = 0;
		for (k=0; k < NUM_ELEMENTS_NOPROBE; k++) {
			if (COMPARE_BUS(bus, k) && COMPARE_ADDR(j, k)) {
				skip = 1;
				break;
			}
		}
		if (skip)
			continue;
#endif
		if (i2c_probe(j) == 0)
			printf(" %02X", j);
	}
	putc ('\n');

#if defined(CONFIG_SYS_I2C_NOPROBES)
	puts ("Excluded chip addresses:");
	for (k=0; k < NUM_ELEMENTS_NOPROBE; k++) {
		if (COMPARE_BUS(bus,k))
			printf(" %02X", NO_PROBE_ADDR(k));
	}
	putc ('\n');
#endif

	return 0;
}

/*
 * Syntax:
 *	i2c loop {i2c_chip} {addr}{.0, .1, .2} [{length}] [{delay}]
 *	{length} - Number of bytes to read
 *	{delay}  - A DECIMAL number and defaults to 1000 uSec
 */
static int do_i2c_loop(cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[])
{
	u_char	chip;
	ulong	alen;
	uint	addr;
	uint	length;
	u_char	bytes[16];
	int	delay;

	if (argc < 3)
		return cmd_usage(cmdtp);

	/*
	 * Chip is always specified.
	 */
	chip = simple_strtoul(argv[1], NULL, 16);

	/*
	 * Address is always specified.
	 */
	addr = simple_strtoul(argv[2], NULL, 16);
	alen = get_alen(argv[2]);
	if (alen > 3)
		return cmd_usage(cmdtp);

	/*
	 * Length is the number of objects, not number of bytes.
	 */
	length = 1;
	length = simple_strtoul(argv[3], NULL, 16);
	if (length > sizeof(bytes))
		length = sizeof(bytes);

	/*
	 * The delay time (uSec) is optional.
	 */
	delay = 1000;
	if (argc > 3)
		delay = simple_strtoul(argv[4], NULL, 10);
	/*
	 * Run the loop...
	 */
	while (1) {
		if (i2c_read(chip, addr, alen, bytes, length) != 0)
			puts ("Error reading the chip.\n");
		udelay(delay);
	}

	/* NOTREACHED */
	return 0;
}

/*
 * The SDRAM command is separately configured because many
 * (most?) embedded boards don't use SDRAM DIMMs.
 */
#if defined(CONFIG_CMD_SDRAM)
static void print_ddr2_tcyc (u_char const b)
{
	printf ("%d.", (b >> 4) & 0x0F);
	switch (b & 0x0F) {
	case 0x0:
	case 0x1:
	case 0x2:
	case 0x3:
	case 0x4:
	case 0x5:
	case 0x6:
	case 0x7:
	case 0x8:
	case 0x9:
		printf ("%d ns\n", b & 0x0F);
		break;
	case 0xA:
		puts ("25 ns\n");
		break;
	case 0xB:
		puts ("33 ns\n");
		break;
	case 0xC:
		puts ("66 ns\n");
		break;
	case 0xD:
		puts ("75 ns\n");
		break;
	default:
		puts ("?? ns\n");
		break;
	}
}

static void decode_bits (u_char const b, char const *str[], int const do_once)
{
	u_char mask;

	for (mask = 0x80; mask != 0x00; mask >>= 1, ++str) {
		if (b & mask) {
			puts (*str);
			if (do_once)
				return;
		}
	}
}

/*
 * Syntax:
 *	i2c sdram {i2c_chip}
 */
static int do_sdram (cmd_tbl_t * cmdtp, int flag, int argc, char * const argv[])
{
	enum { unknown, EDO, SDRAM, DDR2 } type;

	u_char	chip;
	u_char	data[128];
	u_char	cksum;
	int	j;

	static const char *decode_CAS_DDR2[] = {
		" TBD", " 6", " 5", " 4", " 3", " 2", " TBD", " TBD"
	};

	static const char *decode_CAS_default[] = {
		" TBD", " 7", " 6", " 5", " 4", " 3", " 2", " 1"
	};

	static const char *decode_CS_WE_default[] = {
		" TBD", " 6", " 5", " 4", " 3", " 2", " 1", " 0"
	};

	static const char *decode_byte21_default[] = {
		"  TBD (bit 7)\n",
		"  Redundant row address\n",
		"  Differential clock input\n",
		"  Registerd DQMB inputs\n",
		"  Buffered DQMB inputs\n",
		"  On-card PLL\n",
		"  Registered address/control lines\n",
		"  Buffered address/control lines\n"
	};

	static const char *decode_byte22_DDR2[] = {
		"  TBD (bit 7)\n",
		"  TBD (bit 6)\n",
		"  TBD (bit 5)\n",
		"  TBD (bit 4)\n",
		"  TBD (bit 3)\n",
		"  Supports partial array self refresh\n",
		"  Supports 50 ohm ODT\n",
		"  Supports weak driver\n"
	};

	static const char *decode_row_density_DDR2[] = {
		"512 MiB", "256 MiB", "128 MiB", "16 GiB",
		"8 GiB", "4 GiB", "2 GiB", "1 GiB"
	};

	static const char *decode_row_density_default[] = {
		"512 MiB", "256 MiB", "128 MiB", "64 MiB",
		"32 MiB", "16 MiB", "8 MiB", "4 MiB"
	};

	if (argc < 2)
		return cmd_usage(cmdtp);

	/*
	 * Chip is always specified.
	 */
	chip = simple_strtoul (argv[1], NULL, 16);

	if (i2c_read (chip, 0, 1, data, sizeof (data)) != 0) {
		puts ("No SDRAM Serial Presence Detect found.\n");
		return 1;
	}

	cksum = 0;
	for (j = 0; j < 63; j++) {
		cksum += data[j];
	}
	if (cksum != data[63]) {
		printf ("WARNING: Configuration data checksum failure:\n"
			"  is 0x%02x, calculated 0x%02x\n", data[63], cksum);
	}
	printf ("SPD data revision            %d.%d\n",
		(data[62] >> 4) & 0x0F, data[62] & 0x0F);
	printf ("Bytes used                   0x%02X\n", data[0]);
	printf ("Serial memory size           0x%02X\n", 1 << data[1]);

	puts ("Memory type                  ");
	switch (data[2]) {
	case 2:
		type = EDO;
		puts ("EDO\n");
		break;
	case 4:
		type = SDRAM;
		puts ("SDRAM\n");
		break;
	case 8:
		type = DDR2;
		puts ("DDR2\n");
		break;
	default:
		type = unknown;
		puts ("unknown\n");
		break;
	}

	puts ("Row address bits             ");
	if ((data[3] & 0x00F0) == 0)
		printf ("%d\n", data[3] & 0x0F);
	else
		printf ("%d/%d\n", data[3] & 0x0F, (data[3] >> 4) & 0x0F);

	puts ("Column address bits          ");
	if ((data[4] & 0x00F0) == 0)
		printf ("%d\n", data[4] & 0x0F);
	else
		printf ("%d/%d\n", data[4] & 0x0F, (data[4] >> 4) & 0x0F);

	switch (type) {
	case DDR2:
		printf ("Number of ranks              %d\n",
			(data[5] & 0x07) + 1);
		break;
	default:
		printf ("Module rows                  %d\n", data[5]);
		break;
	}

	switch (type) {
	case DDR2:
		printf ("Module data width            %d bits\n", data[6]);
		break;
	default:
		printf ("Module data width            %d bits\n",
			(data[7] << 8) | data[6]);
		break;
	}

	puts ("Interface signal levels      ");
	switch(data[8]) {
		case 0:  puts ("TTL 5.0 V\n");	break;
		case 1:  puts ("LVTTL\n");	break;
		case 2:  puts ("HSTL 1.5 V\n");	break;
		case 3:  puts ("SSTL 3.3 V\n");	break;
		case 4:  puts ("SSTL 2.5 V\n");	break;
		case 5:  puts ("SSTL 1.8 V\n");	break;
		default: puts ("unknown\n");	break;
	}

	switch (type) {
	case DDR2:
		printf ("SDRAM cycle time             ");
		print_ddr2_tcyc (data[9]);
		break;
	default:
		printf ("SDRAM cycle time             %d.%d ns\n",
			(data[9] >> 4) & 0x0F, data[9] & 0x0F);
		break;
	}

	switch (type) {
	case DDR2:
		printf ("SDRAM access time            0.%d%d ns\n",
			(data[10] >> 4) & 0x0F, data[10] & 0x0F);
		break;
	default:
		printf ("SDRAM access time            %d.%d ns\n",
			(data[10] >> 4) & 0x0F, data[10] & 0x0F);
		break;
	}

	puts ("EDC configuration            ");
	switch (data[11]) {
		case 0:  puts ("None\n");	break;
		case 1:  puts ("Parity\n");	break;
		case 2:  puts ("ECC\n");	break;
		default: puts ("unknown\n");	break;
	}

	if ((data[12] & 0x80) == 0)
		puts ("No self refresh, rate        ");
	else
		puts ("Self refresh, rate           ");

	switch(data[12] & 0x7F) {
		case 0:  puts ("15.625 us\n");	break;
		case 1:  puts ("3.9 us\n");	break;
		case 2:  puts ("7.8 us\n");	break;
		case 3:  puts ("31.3 us\n");	break;
		case 4:  puts ("62.5 us\n");	break;
		case 5:  puts ("125 us\n");	break;
		default: puts ("unknown\n");	break;
	}

	switch (type) {
	case DDR2:
		printf ("SDRAM width (primary)        %d\n", data[13]);
		break;
	default:
		printf ("SDRAM width (primary)        %d\n", data[13] & 0x7F);
		if ((data[13] & 0x80) != 0) {
			printf ("  (second bank)              %d\n",
				2 * (data[13] & 0x7F));
		}
		break;
	}

	switch (type) {
	case DDR2:
		if (data[14] != 0)
			printf ("EDC width                    %d\n", data[14]);
		break;
	default:
		if (data[14] != 0) {
			printf ("EDC width                    %d\n",
				data[14] & 0x7F);

			if ((data[14] & 0x80) != 0) {
				printf ("  (second bank)              %d\n",
					2 * (data[14] & 0x7F));
			}
		}
		break;
	}

	if (DDR2 != type) {
		printf ("Min clock delay, back-to-back random column addresses "
			"%d\n", data[15]);
	}

	puts ("Burst length(s)             ");
	if (data[16] & 0x80) puts (" Page");
	if (data[16] & 0x08) puts (" 8");
	if (data[16] & 0x04) puts (" 4");
	if (data[16] & 0x02) puts (" 2");
	if (data[16] & 0x01) puts (" 1");
	putc ('\n');
	printf ("Number of banks              %d\n", data[17]);

	switch (type) {
	case DDR2:
		puts ("CAS latency(s)              ");
		decode_bits (data[18], decode_CAS_DDR2, 0);
		putc ('\n');
		break;
	default:
		puts ("CAS latency(s)              ");
		decode_bits (data[18], decode_CAS_default, 0);
		putc ('\n');
		break;
	}

	if (DDR2 != type) {
		puts ("CS latency(s)               ");
		decode_bits (data[19], decode_CS_WE_default, 0);
		putc ('\n');
	}

	if (DDR2 != type) {
		puts ("WE latency(s)               ");
		decode_bits (data[20], decode_CS_WE_default, 0);
		putc ('\n');
	}

	switch (type) {
	case DDR2:
		puts ("Module attributes:\n");
		if (data[21] & 0x80)
			puts ("  TBD (bit 7)\n");
		if (data[21] & 0x40)
			puts ("  Analysis probe installed\n");
		if (data[21] & 0x20)
			puts ("  TBD (bit 5)\n");
		if (data[21] & 0x10)
			puts ("  FET switch external enable\n");
		printf ("  %d PLLs on DIMM\n", (data[21] >> 2) & 0x03);
		if (data[20] & 0x11) {
			printf ("  %d active registers on DIMM\n",
				(data[21] & 0x03) + 1);
		}
		break;
	default:
		puts ("Module attributes:\n");
		if (!data[21])
			puts ("  (none)\n");
		else
			decode_bits (data[21], decode_byte21_default, 0);
		break;
	}

	switch (type) {
	case DDR2:
		decode_bits (data[22], decode_byte22_DDR2, 0);
		break;
	default:
		puts ("Device attributes:\n");
		if (data[22] & 0x80) puts ("  TBD (bit 7)\n");
		if (data[22] & 0x40) puts ("  TBD (bit 6)\n");
		if (data[22] & 0x20) puts ("  Upper Vcc tolerance 5%\n");
		else                 puts ("  Upper Vcc tolerance 10%\n");
		if (data[22] & 0x10) puts ("  Lower Vcc tolerance 5%\n");
		else                 puts ("  Lower Vcc tolerance 10%\n");
		if (data[22] & 0x08) puts ("  Supports write1/read burst\n");
		if (data[22] & 0x04) puts ("  Supports precharge all\n");
		if (data[22] & 0x02) puts ("  Supports auto precharge\n");
		if (data[22] & 0x01) puts ("  Supports early RAS# precharge\n");
		break;
	}

	switch (type) {
	case DDR2:
		printf ("SDRAM cycle time (2nd highest CAS latency)        ");
		print_ddr2_tcyc (data[23]);
		break;
	default:
		printf ("SDRAM cycle time (2nd highest CAS latency)        %d."
			"%d ns\n", (data[23] >> 4) & 0x0F, data[23] & 0x0F);
		break;
	}

	switch (type) {
	case DDR2:
		printf ("SDRAM access from clock (2nd highest CAS latency) 0."
			"%d%d ns\n", (data[24] >> 4) & 0x0F, data[24] & 0x0F);
		break;
	default:
		printf ("SDRAM access from clock (2nd highest CAS latency) %d."
			"%d ns\n", (data[24] >> 4) & 0x0F, data[24] & 0x0F);
		break;
	}

	switch (type) {
	case DDR2:
		printf ("SDRAM cycle time (3rd highest CAS latency)        ");
		print_ddr2_tcyc (data[25]);
		break;
	default:
		printf ("SDRAM cycle time (3rd highest CAS latency)        %d."
			"%d ns\n", (data[25] >> 4) & 0x0F, data[25] & 0x0F);
		break;
	}

	switch (type) {
	case DDR2:
		printf ("SDRAM access from clock (3rd highest CAS latency) 0."
			"%d%d ns\n", (data[26] >> 4) & 0x0F, data[26] & 0x0F);
		break;
	default:
		printf ("SDRAM access from clock (3rd highest CAS latency) %d."
			"%d ns\n", (data[26] >> 4) & 0x0F, data[26] & 0x0F);
		break;
	}

	switch (type) {
	case DDR2:
		printf ("Minimum row precharge        %d.%02d ns\n",
			(data[27] >> 2) & 0x3F, 25 * (data[27] & 0x03));
		break;
	default:
		printf ("Minimum row precharge        %d ns\n", data[27]);
		break;
	}

	switch (type) {
	case DDR2:
		printf ("Row active to row active min %d.%02d ns\n",
			(data[28] >> 2) & 0x3F, 25 * (data[28] & 0x03));
		break;
	default:
		printf ("Row active to row active min %d ns\n", data[28]);
		break;
	}

	switch (type) {
	case DDR2:
		printf ("RAS to CAS delay min         %d.%02d ns\n",
			(data[29] >> 2) & 0x3F, 25 * (data[29] & 0x03));
		break;
	default:
		printf ("RAS to CAS delay min         %d ns\n", data[29]);
		break;
	}

	printf ("Minimum RAS pulse width      %d ns\n", data[30]);

	switch (type) {
	case DDR2:
		puts ("Density of each row          ");
		decode_bits (data[31], decode_row_density_DDR2, 1);
		putc ('\n');
		break;
	default:
		puts ("Density of each row          ");
		decode_bits (data[31], decode_row_density_default, 1);
		putc ('\n');
		break;
	}

	switch (type) {
	case DDR2:
		puts ("Command and Address setup    ");
		if (data[32] >= 0xA0) {
			printf ("1.%d%d ns\n",
				((data[32] >> 4) & 0x0F) - 10, data[32] & 0x0F);
		} else {
			printf ("0.%d%d ns\n",
				((data[32] >> 4) & 0x0F), data[32] & 0x0F);
		}
		break;
	default:
		printf ("Command and Address setup    %c%d.%d ns\n",
			(data[32] & 0x80) ? '-' : '+',
			(data[32] >> 4) & 0x07, data[32] & 0x0F);
		break;
	}

	switch (type) {
	case DDR2:
		puts ("Command and Address hold     ");
		if (data[33] >= 0xA0) {
			printf ("1.%d%d ns\n",
				((data[33] >> 4) & 0x0F) - 10, data[33] & 0x0F);
		} else {
			printf ("0.%d%d ns\n",
				((data[33] >> 4) & 0x0F), data[33] & 0x0F);
		}
		break;
	default:
		printf ("Command and Address hold     %c%d.%d ns\n",
			(data[33] & 0x80) ? '-' : '+',
			(data[33] >> 4) & 0x07, data[33] & 0x0F);
		break;
	}

	switch (type) {
	case DDR2:
		printf ("Data signal input setup      0.%d%d ns\n",
			(data[34] >> 4) & 0x0F, data[34] & 0x0F);
		break;
	default:
		printf ("Data signal input setup      %c%d.%d ns\n",
			(data[34] & 0x80) ? '-' : '+',
			(data[34] >> 4) & 0x07, data[34] & 0x0F);
		break;
	}

	switch (type) {
	case DDR2:
		printf ("Data signal input hold       0.%d%d ns\n",
			(data[35] >> 4) & 0x0F, data[35] & 0x0F);
		break;
	default:
		printf ("Data signal input hold       %c%d.%d ns\n",
			(data[35] & 0x80) ? '-' : '+',
			(data[35] >> 4) & 0x07, data[35] & 0x0F);
		break;
	}

	puts ("Manufacturer's JEDEC ID      ");
	for (j = 64; j <= 71; j++)
		printf ("%02X ", data[j]);
	putc ('\n');
	printf ("Manufacturing Location       %02X\n", data[72]);
	puts ("Manufacturer's Part Number   ");
	for (j = 73; j <= 90; j++)
		printf ("%02X ", data[j]);
	putc ('\n');
	printf ("Revision Code                %02X %02X\n", data[91], data[92]);
	printf ("Manufacturing Date           %02X %02X\n", data[93], data[94]);
	puts ("Assembly Serial Number       ");
	for (j = 95; j <= 98; j++)
		printf ("%02X ", data[j]);
	putc ('\n');

	if (DDR2 != type) {
		printf ("Speed rating                 PC%d\n",
			data[126] == 0x66 ? 66 : data[126]);
	}
	return 0;
}
#endif

#if defined(CONFIG_I2C_MUX)
static int do_i2c_add_bus(cmd_tbl_t * cmdtp, int flag, int argc, char * const argv[])
{
	int ret=0;

	if (argc == 1) {
		/* show all busses */
		I2C_MUX		*mux;
		I2C_MUX_DEVICE	*device = i2c_mux_devices;

		printf ("Busses reached over muxes:\n");
		while (device != NULL) {
			printf ("Bus ID: %x\n", device->busid);
			printf ("  reached over Mux(es):\n");
			mux = device->mux;
			while (mux != NULL) {
				printf ("    %s@%x ch: %x\n", mux->name, mux->chip, mux->channel);
				mux = mux->next;
			}
			device = device->next;
		}
	} else {
		I2C_MUX_DEVICE *dev;

		dev = i2c_mux_ident_muxstring ((uchar *)argv[1]);
		ret = 0;
	}
	return ret;
}
#endif  /* CONFIG_I2C_MUX */

#if defined(CONFIG_I2C_MULTI_BUS)
static int do_i2c_bus_num(cmd_tbl_t * cmdtp, int flag, int argc, char * const argv[])
{
	int bus_idx, ret=0;

	if (argc == 1)
		/* querying current setting */
		printf("Current bus is %d\n", i2c_get_bus_num());
	else {
		bus_idx = simple_strtoul(argv[1], NULL, 10);
		printf("Setting bus to %d\n", bus_idx);
		ret = i2c_set_bus_num(bus_idx);
		if (ret)
			printf("Failure changing bus number (%d)\n", ret);
	}
	return ret;
}
#endif  /* CONFIG_I2C_MULTI_BUS */

static int do_i2c_bus_speed(cmd_tbl_t * cmdtp, int flag, int argc, char * const argv[])
{
	int speed, ret=0;

	if (argc == 1)
		/* querying current speed */
		printf("Current bus speed=%d\n", i2c_get_bus_speed());
	else {
		speed = simple_strtoul(argv[1], NULL, 10);
		printf("Setting bus speed to %d Hz\n", speed);
		ret = i2c_set_bus_speed(speed);
		if (ret)
			printf("Failure changing bus speed (%d)\n", ret);
	}
	return ret;
}

static int do_i2c_mm(cmd_tbl_t * cmdtp, int flag, int argc, char * const argv[])
{
	return mod_i2c_mem (cmdtp, 1, flag, argc, argv);
}

static int do_i2c_nm(cmd_tbl_t * cmdtp, int flag, int argc, char * const argv[])
{
	return mod_i2c_mem (cmdtp, 0, flag, argc, argv);
}

static int do_i2c_reset(cmd_tbl_t * cmdtp, int flag, int argc, char * const argv[])
{
	i2c_init(CONFIG_SYS_I2C_SPEED, CONFIG_SYS_I2C_SLAVE);
	return 0;
}

static cmd_tbl_t cmd_i2c_sub[] = {
#if defined(CONFIG_I2C_MUX)
	U_BOOT_CMD_MKENT(bus, 1, 1, do_i2c_add_bus, "", ""),
#endif  /* CONFIG_I2C_MUX */
	U_BOOT_CMD_MKENT(crc32, 3, 1, do_i2c_crc, "", ""),
#if defined(CONFIG_I2C_MULTI_BUS)
	U_BOOT_CMD_MKENT(dev, 1, 1, do_i2c_bus_num, "", ""),
#endif  /* CONFIG_I2C_MULTI_BUS */
	U_BOOT_CMD_MKENT(loop, 3, 1, do_i2c_loop, "", ""),
	U_BOOT_CMD_MKENT(md, 3, 1, do_i2c_md, "", ""),
	U_BOOT_CMD_MKENT(mm, 2, 1, do_i2c_mm, "", ""),
	U_BOOT_CMD_MKENT(mw, 3, 1, do_i2c_mw, "", ""),
	U_BOOT_CMD_MKENT(nm, 2, 1, do_i2c_nm, "", ""),
	U_BOOT_CMD_MKENT(probe, 0, 1, do_i2c_probe, "", ""),
	U_BOOT_CMD_MKENT(read, 5, 1, do_i2c_read, "", ""),
	U_BOOT_CMD_MKENT(reset, 0, 1, do_i2c_reset, "", ""),
#if defined(CONFIG_CMD_SDRAM)
	U_BOOT_CMD_MKENT(sdram, 1, 1, do_sdram, "", ""),
#endif
	U_BOOT_CMD_MKENT(speed, 1, 1, do_i2c_bus_speed, "", ""),
};

#ifdef CONFIG_NEEDS_MANUAL_RELOC
void i2c_reloc(void) {
	fixup_cmdtable(cmd_i2c_sub, ARRAY_SIZE(cmd_i2c_sub));
}
#endif

static int do_i2c(cmd_tbl_t * cmdtp, int flag, int argc, char * const argv[])
{
	cmd_tbl_t *c;

	if (argc < 2)
		return cmd_usage(cmdtp);

	/* Strip off leading 'i2c' command argument */
	argc--;
	argv++;

	c = find_cmd_tbl(argv[0], &cmd_i2c_sub[0], ARRAY_SIZE(cmd_i2c_sub));

	if (c)
		return  c->cmd(cmdtp, flag, argc, argv);
	else
		return cmd_usage(cmdtp);
}

/***************************************************/

U_BOOT_CMD(
	i2c, 6, 1, do_i2c,
	"I2C sub-system",
#if defined(CONFIG_I2C_MUX)
	"bus [muxtype:muxaddr:muxchannel] - add a new bus reached over muxes\ni2c "
#endif  /* CONFIG_I2C_MUX */
	"crc32 chip address[.0, .1, .2] count - compute CRC32 checksum\n"
#if defined(CONFIG_I2C_MULTI_BUS)
	"i2c dev [dev] - show or set current I2C bus\n"
#endif  /* CONFIG_I2C_MULTI_BUS */
	"i2c loop chip address[.0, .1, .2] [# of objects] - looping read of device\n"
	"i2c md chip address[.0, .1, .2] [# of objects] - read from I2C device\n"
	"i2c mm chip address[.0, .1, .2] - write to I2C device (auto-incrementing)\n"
	"i2c mw chip address[.0, .1, .2] value [count] - write to I2C device (fill)\n"
	"i2c nm chip address[.0, .1, .2] - write to I2C device (constant address)\n"
	"i2c probe - show devices on the I2C bus\n"
	"i2c read chip address[.0, .1, .2] length memaddress - read to memory \n"
	"i2c reset - re-init the I2C Controller\n"
#if defined(CONFIG_CMD_SDRAM)
	"i2c sdram chip - print SDRAM configuration information\n"
#endif
	"i2c speed [speed] - show or set I2C bus speed"
);

#if defined(CONFIG_I2C_MUX)
static int i2c_mux_add_device(I2C_MUX_DEVICE *dev)
{
	I2C_MUX_DEVICE	*devtmp = i2c_mux_devices;

	if (i2c_mux_devices == NULL) {
		i2c_mux_devices = dev;
		return 0;
	}
	while (devtmp->next != NULL)
		devtmp = devtmp->next;

	devtmp->next = dev;
	return 0;
}

I2C_MUX_DEVICE	*i2c_mux_search_device(int id)
{
	I2C_MUX_DEVICE	*device = i2c_mux_devices;

	while (device != NULL) {
		if (device->busid == id)
			return device;
		device = device->next;
	}
	return NULL;
}

/* searches in the buf from *pos the next ':'.
 * returns:
 *     0 if found (with *pos = where)
 *   < 0 if an error occured
 *   > 0 if the end of buf is reached
 */
static int i2c_mux_search_next (int *pos, uchar	*buf, int len)
{
	while ((buf[*pos] != ':') && (*pos < len)) {
		*pos += 1;
	}
	if (*pos >= len)
		return 1;
	if (buf[*pos] != ':')
		return -1;
	return 0;
}

static int i2c_mux_get_busid (void)
{
	int	tmp = i2c_mux_busid;

	i2c_mux_busid ++;
	return tmp;
}

/* Analyses a Muxstring and immediately sends the
   commands to the muxes. Runs from flash.
 */
int i2c_mux_ident_muxstring_f (uchar *buf)
{
	int	pos = 0;
	int	oldpos;
	int	ret = 0;
	int	len = strlen((char *)buf);
	int	chip;
	uchar	channel;
	int	was = 0;

	while (ret == 0) {
		oldpos = pos;
		/* search name */
		ret = i2c_mux_search_next(&pos, buf, len);
		if (ret != 0)
			printf ("ERROR\n");
		/* search address */
		pos ++;
		oldpos = pos;
		ret = i2c_mux_search_next(&pos, buf, len);
		if (ret != 0)
			printf ("ERROR\n");
		buf[pos] = 0;
		chip = simple_strtoul((char *)&buf[oldpos], NULL, 16);
		buf[pos] = ':';
		/* search channel */
		pos ++;
		oldpos = pos;
		ret = i2c_mux_search_next(&pos, buf, len);
		if (ret < 0)
			printf ("ERROR\n");
		was = 0;
		if (buf[pos] != 0) {
			buf[pos] = 0;
			was = 1;
		}
		channel = simple_strtoul((char *)&buf[oldpos], NULL, 16);
		if (was)
			buf[pos] = ':';
		if (i2c_write(chip, 0, 0, &channel, 1) != 0) {
			printf ("Error setting Mux: chip:%x channel: \
				%x\n", chip, channel);
			return -1;
		}
		pos ++;
		oldpos = pos;

	}

	return 0;
}

/* Analyses a Muxstring and if this String is correct
 * adds a new I2C Bus.
 */
I2C_MUX_DEVICE *i2c_mux_ident_muxstring (uchar *buf)
{
	I2C_MUX_DEVICE	*device;
	I2C_MUX		*mux;
	int	pos = 0;
	int	oldpos;
	int	ret = 0;
	int	len = strlen((char *)buf);
	int	was = 0;

	device = (I2C_MUX_DEVICE *)malloc (sizeof(I2C_MUX_DEVICE));
	device->mux = NULL;
	device->busid = i2c_mux_get_busid ();
	device->next = NULL;
	while (ret == 0) {
		mux = (I2C_MUX *)malloc (sizeof(I2C_MUX));
		mux->next = NULL;
		/* search name of mux */
		oldpos = pos;
		ret = i2c_mux_search_next(&pos, buf, len);
		if (ret != 0)
			printf ("%s no name.\n", __FUNCTION__);
		mux->name = (char *)malloc (pos - oldpos + 1);
		memcpy (mux->name, &buf[oldpos], pos - oldpos);
		mux->name[pos - oldpos] = 0;
		/* search address */
		pos ++;
		oldpos = pos;
		ret = i2c_mux_search_next(&pos, buf, len);
		if (ret != 0)
			printf ("%s no mux address.\n", __FUNCTION__);
		buf[pos] = 0;
		mux->chip = simple_strtoul((char *)&buf[oldpos], NULL, 16);
		buf[pos] = ':';
		/* search channel */
		pos ++;
		oldpos = pos;
		ret = i2c_mux_search_next(&pos, buf, len);
		if (ret < 0)
			printf ("%s no mux channel.\n", __FUNCTION__);
		was = 0;
		if (buf[pos] != 0) {
			buf[pos] = 0;
			was = 1;
		}
		mux->channel = simple_strtoul((char *)&buf[oldpos], NULL, 16);
		if (was)
			buf[pos] = ':';
		if (device->mux == NULL)
			device->mux = mux;
		else {
			I2C_MUX		*muxtmp = device->mux;
			while (muxtmp->next != NULL) {
				muxtmp = muxtmp->next;
			}
			muxtmp->next = mux;
		}
		pos ++;
		oldpos = pos;
	}
	if (ret > 0) {
		/* Add Device */
		i2c_mux_add_device (device);
		return device;
	}

	return NULL;
}

int i2x_mux_select_mux(int bus)
{
	I2C_MUX_DEVICE  *dev;
	I2C_MUX		*mux;

	if ((gd->flags & GD_FLG_RELOC) != GD_FLG_RELOC) {
		/* select Default Mux Bus */
#if defined(CONFIG_SYS_I2C_IVM_BUS)
		i2c_mux_ident_muxstring_f ((uchar *)CONFIG_SYS_I2C_IVM_BUS);
#else
		{
		unsigned char *buf;
		buf = (unsigned char *) getenv("EEprom_ivm");
		if (buf != NULL)
			i2c_mux_ident_muxstring_f (buf);
		}
#endif
		return 0;
	}
	dev = i2c_mux_search_device(bus);
	if (dev == NULL)
		return -1;

	mux = dev->mux;
	while (mux != NULL) {
		/* do deblocking on each level of mux, before mux config */
		i2c_init_board();
		if (i2c_write(mux->chip, 0, 0, &mux->channel, 1) != 0) {
			printf ("Error setting Mux: chip:%x channel: \
				%x\n", mux->chip, mux->channel);
			return -1;
		}
		mux = mux->next;
	}
	/* do deblocking on each level of mux and after mux config */
	i2c_init_board();
	return 0;
}
#endif /* CONFIG_I2C_MUX */
