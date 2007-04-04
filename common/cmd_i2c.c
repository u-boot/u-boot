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
 * Two of the commands (imm and imw) take a byte/word/long modifier
 * (e.g. imm.w specifies the word-length modifier).  This was done to
 * allow manipulating word-length registers.  It was not done on any other
 * commands because it was not deemed useful.
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
 *   CFG_I2C_EEPROM_ADDR_OVERFLOW and using the .1 modifier on the
 *   {addr} field (since .1 is the default, it doesn't actually have to
 *   be specified).  Examples: given a memory chip at I2C chip address
 *   0x50, the following would happen...
 *     imd 50 0 10      display 16 bytes starting at 0x000
 *                      On the bus: <S> A0 00 <E> <S> A1 <rd> ... <rd>
 *     imd 50 100 10    display 16 bytes starting at 0x100
 *                      On the bus: <S> A2 00 <E> <S> A3 <rd> ... <rd>
 *     imd 50 210 10    display 16 bytes starting at 0x210
 *                      On the bus: <S> A4 10 <E> <S> A5 <rd> ... <rd>
 *   This is awfully ugly.  It would be nice if someone would think up
 *   a better way of handling this.
 *
 * Adapted from cmd_mem.c which is copyright Wolfgang Denk (wd@denx.de).
 */

#include <common.h>
#include <command.h>
#include <i2c.h>
#include <asm/byteorder.h>

#if (CONFIG_COMMANDS & CFG_CMD_I2C)


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

#if defined(CFG_I2C_NOPROBES)
#if defined(CONFIG_I2C_MULTI_BUS)
static struct
{
	uchar	bus;
	uchar	addr;
} i2c_no_probes[] = CFG_I2C_NOPROBES;
#define GET_BUS_NUM	i2c_get_bus_num()
#define COMPARE_BUS(b,i)	(i2c_no_probes[(i)].bus == (b))
#define COMPARE_ADDR(a,i)	(i2c_no_probes[(i)].addr == (a))
#define NO_PROBE_ADDR(i)	i2c_no_probes[(i)].addr
#else		/* single bus */
static uchar i2c_no_probes[] = CFG_I2C_NOPROBES;
#define GET_BUS_NUM	0
#define COMPARE_BUS(b,i)	((b) == 0)	/* Make compiler happy */
#define COMPARE_ADDR(a,i)	(i2c_no_probes[(i)] == (a))
#define NO_PROBE_ADDR(i)	i2c_no_probes[(i)]
#endif	/* CONFIG_MULTI_BUS */

#define NUM_ELEMENTS_NOPROBE (sizeof(i2c_no_probes)/sizeof(i2c_no_probes[0]))
#endif

static int
mod_i2c_mem(cmd_tbl_t *cmdtp, int incrflag, int flag, int argc, char *argv[]);
extern int cmd_get_data_size(char* arg, int default_size);

/*
 * Syntax:
 *	imd {i2c_chip} {addr}{.0, .1, .2} {len}
 */
#define DISP_LINE_LEN	16

int do_i2c_md ( cmd_tbl_t *cmdtp, int flag, int argc, char *argv[])
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

	if (argc < 3) {
		printf ("Usage:\n%s\n", cmdtp->usage);
		return 1;
	}

	if ((flag & CMD_FLAG_REPEAT) == 0) {
		/*
		 * New command specified.
		 */
		alen = 1;

		/*
		 * I2C chip address
		 */
		chip = simple_strtoul(argv[1], NULL, 16);

		/*
		 * I2C data address within the chip.  This can be 1 or
		 * 2 bytes long.  Some day it might be 3 bytes long :-).
		 */
		addr = simple_strtoul(argv[2], NULL, 16);
		alen = 1;
		for (j = 0; j < 8; j++) {
			if (argv[2][j] == '.') {
				alen = argv[2][j+1] - '0';
				if (alen > 4) {
					printf ("Usage:\n%s\n", cmdtp->usage);
					return 1;
				}
				break;
			} else if (argv[2][j] == '\0')
				break;
		}

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

int do_i2c_mm ( cmd_tbl_t *cmdtp, int flag, int argc, char *argv[])
{
	return mod_i2c_mem (cmdtp, 1, flag, argc, argv);
}


int do_i2c_nm ( cmd_tbl_t *cmdtp, int flag, int argc, char *argv[])
{
	return mod_i2c_mem (cmdtp, 0, flag, argc, argv);
}

/* Write (fill) memory
 *
 * Syntax:
 *	imw {i2c_chip} {addr}{.0, .1, .2} {data} [{count}]
 */
int do_i2c_mw ( cmd_tbl_t *cmdtp, int flag, int argc, char *argv[])
{
	uchar	chip;
	ulong	addr;
	uint	alen;
	uchar	byte;
	int	count;
	int	j;

	if ((argc < 4) || (argc > 5)) {
		printf ("Usage:\n%s\n", cmdtp->usage);
		return 1;
	}

	/*
 	 * Chip is always specified.
 	 */
	chip = simple_strtoul(argv[1], NULL, 16);

	/*
	 * Address is always specified.
	 */
	addr = simple_strtoul(argv[2], NULL, 16);
	alen = 1;
	for (j = 0; j < 8; j++) {
		if (argv[2][j] == '.') {
			alen = argv[2][j+1] - '0';
			if (alen > 4) {
				printf ("Usage:\n%s\n", cmdtp->usage);
				return 1;
			}
			break;
		} else if (argv[2][j] == '\0')
			break;
	}

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
		 *
		 * On some chips, while the write is in progress, the
		 * chip doesn't respond.  This apparently isn't a
		 * universal feature so we don't take advantage of it.
		 */
/*
 * No write delay with FRAM devices.
 */
#if !defined(CFG_I2C_FRAM)
		udelay(11000);
#endif

#if 0
		for (timeout = 0; timeout < 10; timeout++) {
			udelay(2000);
			if (i2c_probe(chip) == 0)
				break;
		}
#endif
	}

	return (0);
}


/* Calculate a CRC on memory
 *
 * Syntax:
 *	icrc32 {i2c_chip} {addr}{.0, .1, .2} {count}
 */
int do_i2c_crc (cmd_tbl_t *cmdtp, int flag, int argc, char *argv[])
{
	uchar	chip;
	ulong	addr;
	uint	alen;
	int	count;
	uchar	byte;
	ulong	crc;
	ulong	err;
	int	j;

	if (argc < 4) {
		printf ("Usage:\n%s\n", cmdtp->usage);
		return 1;
	}

	/*
 	 * Chip is always specified.
 	 */
	chip = simple_strtoul(argv[1], NULL, 16);

	/*
	 * Address is always specified.
	 */
	addr = simple_strtoul(argv[2], NULL, 16);
	alen = 1;
	for (j = 0; j < 8; j++) {
		if (argv[2][j] == '.') {
			alen = argv[2][j+1] - '0';
			if (alen > 4) {
				printf ("Usage:\n%s\n", cmdtp->usage);
				return 1;
			}
			break;
		} else if (argv[2][j] == '\0')
			break;
	}

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
 *	imm{.b, .w, .l} {i2c_chip} {addr}{.0, .1, .2}
 *	inm{.b, .w, .l} {i2c_chip} {addr}{.0, .1, .2}
 */

static int
mod_i2c_mem(cmd_tbl_t *cmdtp, int incrflag, int flag, int argc, char *argv[])
{
	uchar	chip;
	ulong	addr;
	uint	alen;
	ulong	data;
	int	size = 1;
	int	nbytes;
	int	j;
	extern char console_buffer[];

	if (argc != 3) {
		printf ("Usage:\n%s\n", cmdtp->usage);
		return 1;
	}

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
		alen = 1;
		for (j = 0; j < 8; j++) {
			if (argv[2][j] == '.') {
				alen = argv[2][j+1] - '0';
				if (alen > 4) {
					printf ("Usage:\n%s\n", cmdtp->usage);
					return 1;
				}
				break;
			} else if (argv[2][j] == '\0')
				break;
		}
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
#ifdef CFG_EEPROM_PAGE_WRITE_DELAY_MS
				udelay(CFG_EEPROM_PAGE_WRITE_DELAY_MS * 1000);
#endif
				if (incrflag)
					addr += size;
			}
		}
	} while (nbytes);

	chip = i2c_mm_last_chip;
	addr = i2c_mm_last_addr;
	alen = i2c_mm_last_alen;

	return 0;
}

/*
 * Syntax:
 *	iprobe {addr}{.0, .1, .2}
 */
int do_i2c_probe (cmd_tbl_t *cmdtp, int flag, int argc, char *argv[])
{
	int j;
#if defined(CFG_I2C_NOPROBES)
	int k, skip;
	uchar bus = GET_BUS_NUM;
#endif	/* NOPROBES */

	puts ("Valid chip addresses:");
	for (j = 0; j < 128; j++) {
#if defined(CFG_I2C_NOPROBES)
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

#if defined(CFG_I2C_NOPROBES)
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
 *	iloop {i2c_chip} {addr}{.0, .1, .2} [{length}] [{delay}]
 *	{length} - Number of bytes to read
 *	{delay}  - A DECIMAL number and defaults to 1000 uSec
 */
int do_i2c_loop(cmd_tbl_t *cmdtp, int flag, int argc, char *argv[])
{
	u_char	chip;
	ulong	alen;
	uint	addr;
	uint	length;
	u_char	bytes[16];
	int	delay;
	int	j;

	if (argc < 3) {
		printf ("Usage:\n%s\n", cmdtp->usage);
		return 1;
	}

	/*
	 * Chip is always specified.
	 */
	chip = simple_strtoul(argv[1], NULL, 16);

	/*
	 * Address is always specified.
	 */
	addr = simple_strtoul(argv[2], NULL, 16);
	alen = 1;
	for (j = 0; j < 8; j++) {
		if (argv[2][j] == '.') {
			alen = argv[2][j+1] - '0';
			if (alen > 4) {
				printf ("Usage:\n%s\n", cmdtp->usage);
				return 1;
			}
			break;
		} else if (argv[2][j] == '\0')
			break;
	}

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
#if (CONFIG_COMMANDS & CFG_CMD_SDRAM)

/*
 * Syntax:
 *	sdram {i2c_chip}
 */
int do_sdram  ( cmd_tbl_t *cmdtp, int flag, int argc, char *argv[])
{
	u_char	chip;
	u_char	data[128];
	u_char	cksum;
	int	j;

	if (argc < 2) {
		printf ("Usage:\n%s\n", cmdtp->usage);
		return 1;
	}
	/*
	 * Chip is always specified.
 	 */
	chip = simple_strtoul(argv[1], NULL, 16);

	if (i2c_read(chip, 0, 1, data, sizeof(data)) != 0) {
		puts ("No SDRAM Serial Presence Detect found.\n");
		return 1;
	}

	cksum = 0;
	for (j = 0; j < 63; j++) {
		cksum += data[j];
	}
	if (cksum != data[63]) {
		printf ("WARNING: Configuration data checksum failure:\n"
			"  is 0x%02x, calculated 0x%02x\n",
			data[63], cksum);
	}
	printf("SPD data revision            %d.%d\n",
		(data[62] >> 4) & 0x0F, data[62] & 0x0F);
	printf("Bytes used                   0x%02X\n", data[0]);
	printf("Serial memory size           0x%02X\n", 1 << data[1]);
	puts ("Memory type                  ");
	switch(data[2]) {
		case 2:  puts ("EDO\n");	break;
		case 4:  puts ("SDRAM\n");	break;
		case 8:  puts ("DDR2\n");	break;
		default: puts ("unknown\n");	break;
	}
	puts ("Row address bits             ");
	if ((data[3] & 0x00F0) == 0)
		printf("%d\n", data[3] & 0x0F);
	else
		printf("%d/%d\n", data[3] & 0x0F, (data[3] >> 4) & 0x0F);
	puts ("Column address bits          ");
	if ((data[4] & 0x00F0) == 0)
		printf("%d\n", data[4] & 0x0F);
	else
		printf("%d/%d\n", data[4] & 0x0F, (data[4] >> 4) & 0x0F);
	printf("Module rows                  %d\n", data[5]);
	printf("Module data width            %d bits\n", (data[7] << 8) | data[6]);
	puts ("Interface signal levels      ");
	switch(data[8]) {
		case 0:  puts ("5.0v/TTL\n");	break;
		case 1:  puts ("LVTTL\n");	break;
		case 2:  puts ("HSTL 1.5\n");	break;
		case 3:  puts ("SSTL 3.3\n");	break;
		case 4:  puts ("SSTL 2.5\n");	break;
		case 5:  puts ("SSTL 1.8\n");	break;
		default: puts ("unknown\n");	break;
	}
	printf("SDRAM cycle time             %d.%d nS\n",
		(data[9] >> 4) & 0x0F, data[9] & 0x0F);
	printf("SDRAM access time            %d.%d nS\n",
		(data[10] >> 4) & 0x0F, data[10] & 0x0F);
	puts ("EDC configuration            ");
	switch(data[11]) {
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
		case 0:  puts ("15.625uS\n");	break;
		case 1:  puts ("3.9uS\n");	break;
		case 2:  puts ("7.8uS\n");	break;
		case 3:  puts ("31.3uS\n");	break;
		case 4:  puts ("62.5uS\n");	break;
		case 5:  puts ("125uS\n");	break;
		default: puts ("unknown\n");	break;
	}
	printf("SDRAM width (primary)        %d\n", data[13] & 0x7F);
	if ((data[13] & 0x80) != 0) {
		printf("  (second bank)              %d\n",
			2 * (data[13] & 0x7F));
	}
	if (data[14] != 0) {
		printf("EDC width                    %d\n",
			data[14] & 0x7F);
		if ((data[14] & 0x80) != 0)
			printf("  (second bank)              %d\n",
				2 * (data[14] & 0x7F));
	}
	printf("Min clock delay, back-to-back random column addresses %d\n",
		data[15]);
	puts ("Burst length(s)             ");
	if (data[16] & 0x80) puts (" Page");
	if (data[16] & 0x08) puts (" 8");
	if (data[16] & 0x04) puts (" 4");
	if (data[16] & 0x02) puts (" 2");
	if (data[16] & 0x01) puts (" 1");
	putc ('\n');
	printf("Number of banks              %d\n", data[17]);
	puts ("CAS latency(s)              ");
	if (data[18] & 0x80) puts (" TBD");
	if (data[18] & 0x40) puts (" 7");
	if (data[18] & 0x20) puts (" 6");
	if (data[18] & 0x10) puts (" 5");
	if (data[18] & 0x08) puts (" 4");
	if (data[18] & 0x04) puts (" 3");
	if (data[18] & 0x02) puts (" 2");
	if (data[18] & 0x01) puts (" 1");
	putc ('\n');
	puts ("CS latency(s)               ");
	if (data[19] & 0x80) puts (" TBD");
	if (data[19] & 0x40) puts (" 6");
	if (data[19] & 0x20) puts (" 5");
	if (data[19] & 0x10) puts (" 4");
	if (data[19] & 0x08) puts (" 3");
	if (data[19] & 0x04) puts (" 2");
	if (data[19] & 0x02) puts (" 1");
	if (data[19] & 0x01) puts (" 0");
	putc ('\n');
	puts ("WE latency(s)               ");
	if (data[20] & 0x80) puts (" TBD");
	if (data[20] & 0x40) puts (" 6");
	if (data[20] & 0x20) puts (" 5");
	if (data[20] & 0x10) puts (" 4");
	if (data[20] & 0x08) puts (" 3");
	if (data[20] & 0x04) puts (" 2");
	if (data[20] & 0x02) puts (" 1");
	if (data[20] & 0x01) puts (" 0");
	putc ('\n');
	puts ("Module attributes:\n");
	if (!data[21])       puts ("  (none)\n");
	if (data[21] & 0x80) puts ("  TBD (bit 7)\n");
	if (data[21] & 0x40) puts ("  Redundant row address\n");
	if (data[21] & 0x20) puts ("  Differential clock input\n");
	if (data[21] & 0x10) puts ("  Registerd DQMB inputs\n");
	if (data[21] & 0x08) puts ("  Buffered DQMB inputs\n");
	if (data[21] & 0x04) puts ("  On-card PLL\n");
	if (data[21] & 0x02) puts ("  Registered address/control lines\n");
	if (data[21] & 0x01) puts ("  Buffered address/control lines\n");
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
	printf("SDRAM cycle time (2nd highest CAS latency)        %d.%d nS\n",
		(data[23] >> 4) & 0x0F, data[23] & 0x0F);
	printf("SDRAM access from clock (2nd highest CAS latency) %d.%d nS\n",
		(data[24] >> 4) & 0x0F, data[24] & 0x0F);
	printf("SDRAM cycle time (3rd highest CAS latency)        %d.%d nS\n",
		(data[25] >> 4) & 0x0F, data[25] & 0x0F);
	printf("SDRAM access from clock (3rd highest CAS latency) %d.%d nS\n",
		(data[26] >> 4) & 0x0F, data[26] & 0x0F);
	printf("Minimum row precharge        %d nS\n", data[27]);
	printf("Row active to row active min %d nS\n", data[28]);
	printf("RAS to CAS delay min         %d nS\n", data[29]);
	printf("Minimum RAS pulse width      %d nS\n", data[30]);
	puts ("Density of each row         ");
	if (data[31] & 0x80) puts (" 512");
	if (data[31] & 0x40) puts (" 256");
	if (data[31] & 0x20) puts (" 128");
	if (data[31] & 0x10) puts (" 64");
	if (data[31] & 0x08) puts (" 32");
	if (data[31] & 0x04) puts (" 16");
	if (data[31] & 0x02) puts (" 8");
	if (data[31] & 0x01) puts (" 4");
	puts ("MByte\n");
	printf("Command and Address setup    %c%d.%d nS\n",
		(data[32] & 0x80) ? '-' : '+',
		(data[32] >> 4) & 0x07, data[32] & 0x0F);
	printf("Command and Address hold     %c%d.%d nS\n",
		(data[33] & 0x80) ? '-' : '+',
		(data[33] >> 4) & 0x07, data[33] & 0x0F);
	printf("Data signal input setup      %c%d.%d nS\n",
		(data[34] & 0x80) ? '-' : '+',
		(data[34] >> 4) & 0x07, data[34] & 0x0F);
	printf("Data signal input hold       %c%d.%d nS\n",
		(data[35] & 0x80) ? '-' : '+',
		(data[35] >> 4) & 0x07, data[35] & 0x0F);
	puts ("Manufacturer's JEDEC ID      ");
	for (j = 64; j <= 71; j++)
		printf("%02X ", data[j]);
	putc ('\n');
	printf("Manufacturing Location       %02X\n", data[72]);
	puts ("Manufacturer's Part Number   ");
	for (j = 73; j <= 90; j++)
		printf("%02X ", data[j]);
	putc ('\n');
	printf("Revision Code                %02X %02X\n", data[91], data[92]);
	printf("Manufacturing Date           %02X %02X\n", data[93], data[94]);
	puts ("Assembly Serial Number       ");
	for (j = 95; j <= 98; j++)
		printf("%02X ", data[j]);
	putc ('\n');
	printf("Speed rating                 PC%d\n",
		data[126] == 0x66 ? 66 : data[126]);

	return 0;
}
#endif	/* CFG_CMD_SDRAM */

#if defined(CONFIG_I2C_CMD_TREE)
#if defined(CONFIG_I2C_MULTI_BUS)
int do_i2c_bus_num(cmd_tbl_t * cmdtp, int flag, int argc, char *argv[])
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

int do_i2c_bus_speed(cmd_tbl_t * cmdtp, int flag, int argc, char *argv[])
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

int do_i2c(cmd_tbl_t * cmdtp, int flag, int argc, char *argv[])
{
#if defined(CONFIG_I2C_MULTI_BUS)
	if (!strncmp(argv[1], "de", 2))
		return do_i2c_bus_num(cmdtp, flag, --argc, ++argv);
#endif  /* CONFIG_I2C_MULTI_BUS */
	if (!strncmp(argv[1], "sp", 2))
		return do_i2c_bus_speed(cmdtp, flag, --argc, ++argv);
	if (!strncmp(argv[1], "md", 2))
		return do_i2c_md(cmdtp, flag, --argc, ++argv);
	if (!strncmp(argv[1], "mm", 2))
		return do_i2c_mm(cmdtp, flag, --argc, ++argv);
	if (!strncmp(argv[1], "mw", 2))
		return do_i2c_mw(cmdtp, flag, --argc, ++argv);
	if (!strncmp(argv[1], "nm", 2))
		return do_i2c_nm(cmdtp, flag, --argc, ++argv);
	if (!strncmp(argv[1], "cr", 2))
		return do_i2c_crc(cmdtp, flag, --argc, ++argv);
	if (!strncmp(argv[1], "pr", 2))
		return do_i2c_probe(cmdtp, flag, --argc, ++argv);
	if (!strncmp(argv[1], "lo", 2))
		return do_i2c_loop(cmdtp, flag, --argc, ++argv);
#if (CONFIG_COMMANDS & CFG_CMD_SDRAM)
	if (!strncmp(argv[1], "sd", 2))
		return do_sdram(cmdtp, flag, --argc, ++argv);
#endif	/* CFG_CMD_SDRAM */
	else
		printf ("Usage:\n%s\n", cmdtp->usage);
	return 0;
}
#endif  /* CONFIG_I2C_CMD_TREE */

/***************************************************/

#if defined(CONFIG_I2C_CMD_TREE)
U_BOOT_CMD(
	i2c, 6, 1, do_i2c,
 	"i2c     - I2C sub-system\n",
#if defined(CONFIG_I2C_MULTI_BUS)
	"dev [dev] - show or set current I2C bus\n"
#endif  /* CONFIG_I2C_MULTI_BUS */
	"i2c speed [speed] - show or set I2C bus speed\n"
	"i2c md chip address[.0, .1, .2] [# of objects] - read from I2C device\n"
	"i2c mm chip address[.0, .1, .2] - write to I2C device (auto-incrementing)\n"
	"i2c mw chip address[.0, .1, .2] value [count] - write to I2C device (fill)\n"
	"i2c nm chip address[.0, .1, .2] - write to I2C device (constant address)\n"
	"i2c crc32 chip address[.0, .1, .2] count - compute CRC32 checksum\n"
	"i2c probe - show devices on the I2C bus\n"
	"i2c loop chip address[.0, .1, .2] [# of objects] - looping read of device\n"
#if (CONFIG_COMMANDS & CFG_CMD_SDRAM)
	"i2c sdram chip - print SDRAM configuration information\n"
#endif  /* CFG_CMD_SDRAM */
);
#endif /* CONFIG_I2C_CMD_TREE */
U_BOOT_CMD(
	imd,	4,	1,	do_i2c_md,		\
	"imd     - i2c memory display\n",				\
	"chip address[.0, .1, .2] [# of objects]\n    - i2c memory display\n" \
);

U_BOOT_CMD(
 	imm,	3,	1,	do_i2c_mm,
	"imm     - i2c memory modify (auto-incrementing)\n",
	"chip address[.0, .1, .2]\n"
	"    - memory modify, auto increment address\n"
);
U_BOOT_CMD(
	inm,	3,	1,	do_i2c_nm,
	"inm     - memory modify (constant address)\n",
	"chip address[.0, .1, .2]\n    - memory modify, read and keep address\n"
);

U_BOOT_CMD(
	imw,	5,	1,	do_i2c_mw,
	"imw     - memory write (fill)\n",
	"chip address[.0, .1, .2] value [count]\n    - memory write (fill)\n"
);

U_BOOT_CMD(
	icrc32,	5,	1,	do_i2c_crc,
	"icrc32  - checksum calculation\n",
	"chip address[.0, .1, .2] count\n    - compute CRC32 checksum\n"
);

U_BOOT_CMD(
	iprobe,	1,	1,	do_i2c_probe,
	"iprobe  - probe to discover valid I2C chip addresses\n",
	"\n    -discover valid I2C chip addresses\n"
);

/*
 * Require full name for "iloop" because it is an infinite loop!
 */
U_BOOT_CMD(
	iloop,	5,	1,	do_i2c_loop,
	"iloop   - infinite loop on address range\n",
	"chip address[.0, .1, .2] [# of objects]\n"
	"    - loop, reading a set of addresses\n"
);

#if (CONFIG_COMMANDS & CFG_CMD_SDRAM)
U_BOOT_CMD(
	isdram,	2,	1,	do_sdram,
	"isdram  - print SDRAM configuration information\n",
	"chip\n    - print SDRAM configuration information\n"
	"      (valid chip values 50..57)\n"
);
#endif

#endif	/* CFG_CMD_I2C */
