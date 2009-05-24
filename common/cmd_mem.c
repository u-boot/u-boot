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

/*
 * Memory Functions
 *
 * Copied from FADS ROM, Dan Malek (dmalek@jlc.net)
 */

#include <common.h>
#include <command.h>
#ifdef CONFIG_HAS_DATAFLASH
#include <dataflash.h>
#endif
#include <watchdog.h>

#ifdef	CMD_MEM_DEBUG
#define	PRINTF(fmt,args...)	printf (fmt ,##args)
#else
#define PRINTF(fmt,args...)
#endif

static int mod_mem(cmd_tbl_t *, int, int, int, char *[]);

/* Display values from last command.
 * Memory modify remembered values are different from display memory.
 */
uint	dp_last_addr, dp_last_size;
uint	dp_last_length = 0x40;
uint	mm_last_addr, mm_last_size;

static	ulong	base_address = 0;

/* Memory Display
 *
 * Syntax:
 *	md{.b, .w, .l} {addr} {len}
 */
#define DISP_LINE_LEN	16
int do_mem_md ( cmd_tbl_t *cmdtp, int flag, int argc, char *argv[])
{
	ulong	addr, length;
#if defined(CONFIG_HAS_DATAFLASH)
	ulong	nbytes, linebytes;
#endif
	int	size;
	int rc = 0;

	/* We use the last specified parameters, unless new ones are
	 * entered.
	 */
	addr = dp_last_addr;
	size = dp_last_size;
	length = dp_last_length;

	if (argc < 2) {
		cmd_usage(cmdtp);
		return 1;
	}

	if ((flag & CMD_FLAG_REPEAT) == 0) {
		/* New command specified.  Check for a size specification.
		 * Defaults to long if no or incorrect specification.
		 */
		if ((size = cmd_get_data_size(argv[0], 4)) < 0)
			return 1;

		/* Address is specified since argc > 1
		*/
		addr = simple_strtoul(argv[1], NULL, 16);
		addr += base_address;

		/* If another parameter, it is the length to display.
		 * Length is the number of objects, not number of bytes.
		 */
		if (argc > 2)
			length = simple_strtoul(argv[2], NULL, 16);
	}

#if defined(CONFIG_HAS_DATAFLASH)
	/* Print the lines.
	 *
	 * We buffer all read data, so we can make sure data is read only
	 * once, and all accesses are with the specified bus width.
	 */
	nbytes = length * size;
	do {
		char	linebuf[DISP_LINE_LEN];
		void* p;
		linebytes = (nbytes>DISP_LINE_LEN)?DISP_LINE_LEN:nbytes;

		rc = read_dataflash(addr, (linebytes/size)*size, linebuf);
		p = (rc == DATAFLASH_OK) ? linebuf : (void*)addr;
		print_buffer(addr, p, size, linebytes/size, DISP_LINE_LEN/size);

		nbytes -= linebytes;
		addr += linebytes;
		if (ctrlc()) {
			rc = 1;
			break;
		}
	} while (nbytes > 0);
#else

# if defined(CONFIG_BLACKFIN)
	/* See if we're trying to display L1 inst */
	if (addr_bfin_on_chip_mem(addr)) {
		char linebuf[DISP_LINE_LEN];
		ulong linebytes, nbytes = length * size;
		do {
			linebytes = (nbytes > DISP_LINE_LEN) ? DISP_LINE_LEN : nbytes;
			memcpy(linebuf, (void *)addr, linebytes);
			print_buffer(addr, linebuf, size, linebytes/size, DISP_LINE_LEN/size);

			nbytes -= linebytes;
			addr += linebytes;
			if (ctrlc()) {
				rc = 1;
				break;
			}
		} while (nbytes > 0);
	} else
# endif

	{
		/* Print the lines. */
		print_buffer(addr, (void*)addr, size, length, DISP_LINE_LEN/size);
		addr += size*length;
	}
#endif

	dp_last_addr = addr;
	dp_last_length = length;
	dp_last_size = size;
	return (rc);
}

int do_mem_mm ( cmd_tbl_t *cmdtp, int flag, int argc, char *argv[])
{
	return mod_mem (cmdtp, 1, flag, argc, argv);
}
int do_mem_nm ( cmd_tbl_t *cmdtp, int flag, int argc, char *argv[])
{
	return mod_mem (cmdtp, 0, flag, argc, argv);
}

int do_mem_mw ( cmd_tbl_t *cmdtp, int flag, int argc, char *argv[])
{
	ulong	addr, writeval, count;
	int	size;

	if ((argc < 3) || (argc > 4)) {
		cmd_usage(cmdtp);
		return 1;
	}

	/* Check for size specification.
	*/
	if ((size = cmd_get_data_size(argv[0], 4)) < 1)
		return 1;

	/* Address is specified since argc > 1
	*/
	addr = simple_strtoul(argv[1], NULL, 16);
	addr += base_address;

	/* Get the value to write.
	*/
	writeval = simple_strtoul(argv[2], NULL, 16);

	/* Count ? */
	if (argc == 4) {
		count = simple_strtoul(argv[3], NULL, 16);
	} else {
		count = 1;
	}

	while (count-- > 0) {
		if (size == 4)
			*((ulong  *)addr) = (ulong )writeval;
		else if (size == 2)
			*((ushort *)addr) = (ushort)writeval;
		else
			*((u_char *)addr) = (u_char)writeval;
		addr += size;
	}
	return 0;
}

#ifdef CONFIG_MX_CYCLIC
int do_mem_mdc ( cmd_tbl_t *cmdtp, int flag, int argc, char *argv[])
{
	int i;
	ulong count;

	if (argc < 4) {
		cmd_usage(cmdtp);
		return 1;
	}

	count = simple_strtoul(argv[3], NULL, 10);

	for (;;) {
		do_mem_md (NULL, 0, 3, argv);

		/* delay for <count> ms... */
		for (i=0; i<count; i++)
			udelay (1000);

		/* check for ctrl-c to abort... */
		if (ctrlc()) {
			puts("Abort\n");
			return 0;
		}
	}

	return 0;
}

int do_mem_mwc ( cmd_tbl_t *cmdtp, int flag, int argc, char *argv[])
{
	int i;
	ulong count;

	if (argc < 4) {
		cmd_usage(cmdtp);
		return 1;
	}

	count = simple_strtoul(argv[3], NULL, 10);

	for (;;) {
		do_mem_mw (NULL, 0, 3, argv);

		/* delay for <count> ms... */
		for (i=0; i<count; i++)
			udelay (1000);

		/* check for ctrl-c to abort... */
		if (ctrlc()) {
			puts("Abort\n");
			return 0;
		}
	}

	return 0;
}
#endif /* CONFIG_MX_CYCLIC */

int do_mem_cmp (cmd_tbl_t *cmdtp, int flag, int argc, char *argv[])
{
	ulong	addr1, addr2, count, ngood;
	int	size;
	int     rcode = 0;

	if (argc != 4) {
		cmd_usage(cmdtp);
		return 1;
	}

	/* Check for size specification.
	*/
	if ((size = cmd_get_data_size(argv[0], 4)) < 0)
		return 1;

	addr1 = simple_strtoul(argv[1], NULL, 16);
	addr1 += base_address;

	addr2 = simple_strtoul(argv[2], NULL, 16);
	addr2 += base_address;

	count = simple_strtoul(argv[3], NULL, 16);

#ifdef CONFIG_HAS_DATAFLASH
	if (addr_dataflash(addr1) | addr_dataflash(addr2)){
		puts ("Comparison with DataFlash space not supported.\n\r");
		return 0;
	}
#endif

#ifdef CONFIG_BLACKFIN
	if (addr_bfin_on_chip_mem(addr1) || addr_bfin_on_chip_mem(addr2)) {
		puts ("Comparison with L1 instruction memory not supported.\n\r");
		return 0;
	}
#endif

	ngood = 0;

	while (count-- > 0) {
		if (size == 4) {
			ulong word1 = *(ulong *)addr1;
			ulong word2 = *(ulong *)addr2;
			if (word1 != word2) {
				printf("word at 0x%08lx (0x%08lx) "
					"!= word at 0x%08lx (0x%08lx)\n",
					addr1, word1, addr2, word2);
				rcode = 1;
				break;
			}
		}
		else if (size == 2) {
			ushort hword1 = *(ushort *)addr1;
			ushort hword2 = *(ushort *)addr2;
			if (hword1 != hword2) {
				printf("halfword at 0x%08lx (0x%04x) "
					"!= halfword at 0x%08lx (0x%04x)\n",
					addr1, hword1, addr2, hword2);
				rcode = 1;
				break;
			}
		}
		else {
			u_char byte1 = *(u_char *)addr1;
			u_char byte2 = *(u_char *)addr2;
			if (byte1 != byte2) {
				printf("byte at 0x%08lx (0x%02x) "
					"!= byte at 0x%08lx (0x%02x)\n",
					addr1, byte1, addr2, byte2);
				rcode = 1;
				break;
			}
		}
		ngood++;
		addr1 += size;
		addr2 += size;
	}

	printf("Total of %ld %s%s were the same\n",
		ngood, size == 4 ? "word" : size == 2 ? "halfword" : "byte",
		ngood == 1 ? "" : "s");
	return rcode;
}

int do_mem_cp ( cmd_tbl_t *cmdtp, int flag, int argc, char *argv[])
{
	ulong	addr, dest, count;
	int	size;

	if (argc != 4) {
		cmd_usage(cmdtp);
		return 1;
	}

	/* Check for size specification.
	*/
	if ((size = cmd_get_data_size(argv[0], 4)) < 0)
		return 1;

	addr = simple_strtoul(argv[1], NULL, 16);
	addr += base_address;

	dest = simple_strtoul(argv[2], NULL, 16);
	dest += base_address;

	count = simple_strtoul(argv[3], NULL, 16);

	if (count == 0) {
		puts ("Zero length ???\n");
		return 1;
	}

#ifndef CONFIG_SYS_NO_FLASH
	/* check if we are copying to Flash */
	if ( (addr2info(dest) != NULL)
#ifdef CONFIG_HAS_DATAFLASH
	   && (!addr_dataflash(dest))
#endif
	   ) {
		int rc;

		puts ("Copy to Flash... ");

		rc = flash_write ((char *)addr, dest, count*size);
		if (rc != 0) {
			flash_perror (rc);
			return (1);
		}
		puts ("done\n");
		return 0;
	}
#endif

#ifdef CONFIG_HAS_DATAFLASH
	/* Check if we are copying from RAM or Flash to DataFlash */
	if (addr_dataflash(dest) && !addr_dataflash(addr)){
		int rc;

		puts ("Copy to DataFlash... ");

		rc = write_dataflash (dest, addr, count*size);

		if (rc != 1) {
			dataflash_perror (rc);
			return (1);
		}
		puts ("done\n");
		return 0;
	}

	/* Check if we are copying from DataFlash to RAM */
	if (addr_dataflash(addr) && !addr_dataflash(dest)
#ifndef CONFIG_SYS_NO_FLASH
				 && (addr2info(dest) == NULL)
#endif
	   ){
		int rc;
		rc = read_dataflash(addr, count * size, (char *) dest);
		if (rc != 1) {
			dataflash_perror (rc);
			return (1);
		}
		return 0;
	}

	if (addr_dataflash(addr) && addr_dataflash(dest)){
		puts ("Unsupported combination of source/destination.\n\r");
		return 1;
	}
#endif

#ifdef CONFIG_BLACKFIN
	/* See if we're copying to/from L1 inst */
	if (addr_bfin_on_chip_mem(dest) || addr_bfin_on_chip_mem(addr)) {
		memcpy((void *)dest, (void *)addr, count * size);
		return 0;
	}
#endif

	while (count-- > 0) {
		if (size == 4)
			*((ulong  *)dest) = *((ulong  *)addr);
		else if (size == 2)
			*((ushort *)dest) = *((ushort *)addr);
		else
			*((u_char *)dest) = *((u_char *)addr);
		addr += size;
		dest += size;
	}
	return 0;
}

int do_mem_base (cmd_tbl_t *cmdtp, int flag, int argc, char *argv[])
{
	if (argc > 1) {
		/* Set new base address.
		*/
		base_address = simple_strtoul(argv[1], NULL, 16);
	}
	/* Print the current base address.
	*/
	printf("Base Address: 0x%08lx\n", base_address);
	return 0;
}

int do_mem_loop (cmd_tbl_t *cmdtp, int flag, int argc, char *argv[])
{
	ulong	addr, length, i, junk;
	int	size;
	volatile uint	*longp;
	volatile ushort *shortp;
	volatile u_char	*cp;

	if (argc < 3) {
		cmd_usage(cmdtp);
		return 1;
	}

	/* Check for a size spefication.
	 * Defaults to long if no or incorrect specification.
	 */
	if ((size = cmd_get_data_size(argv[0], 4)) < 0)
		return 1;

	/* Address is always specified.
	*/
	addr = simple_strtoul(argv[1], NULL, 16);

	/* Length is the number of objects, not number of bytes.
	*/
	length = simple_strtoul(argv[2], NULL, 16);

	/* We want to optimize the loops to run as fast as possible.
	 * If we have only one object, just run infinite loops.
	 */
	if (length == 1) {
		if (size == 4) {
			longp = (uint *)addr;
			for (;;)
				i = *longp;
		}
		if (size == 2) {
			shortp = (ushort *)addr;
			for (;;)
				i = *shortp;
		}
		cp = (u_char *)addr;
		for (;;)
			i = *cp;
	}

	if (size == 4) {
		for (;;) {
			longp = (uint *)addr;
			i = length;
			while (i-- > 0)
				junk = *longp++;
		}
	}
	if (size == 2) {
		for (;;) {
			shortp = (ushort *)addr;
			i = length;
			while (i-- > 0)
				junk = *shortp++;
		}
	}
	for (;;) {
		cp = (u_char *)addr;
		i = length;
		while (i-- > 0)
			junk = *cp++;
	}
}

#ifdef CONFIG_LOOPW
int do_mem_loopw (cmd_tbl_t *cmdtp, int flag, int argc, char *argv[])
{
	ulong	addr, length, i, data;
	int	size;
	volatile uint	*longp;
	volatile ushort *shortp;
	volatile u_char	*cp;

	if (argc < 4) {
		cmd_usage(cmdtp);
		return 1;
	}

	/* Check for a size spefication.
	 * Defaults to long if no or incorrect specification.
	 */
	if ((size = cmd_get_data_size(argv[0], 4)) < 0)
		return 1;

	/* Address is always specified.
	*/
	addr = simple_strtoul(argv[1], NULL, 16);

	/* Length is the number of objects, not number of bytes.
	*/
	length = simple_strtoul(argv[2], NULL, 16);

	/* data to write */
	data = simple_strtoul(argv[3], NULL, 16);

	/* We want to optimize the loops to run as fast as possible.
	 * If we have only one object, just run infinite loops.
	 */
	if (length == 1) {
		if (size == 4) {
			longp = (uint *)addr;
			for (;;)
				*longp = data;
					}
		if (size == 2) {
			shortp = (ushort *)addr;
			for (;;)
				*shortp = data;
		}
		cp = (u_char *)addr;
		for (;;)
			*cp = data;
	}

	if (size == 4) {
		for (;;) {
			longp = (uint *)addr;
			i = length;
			while (i-- > 0)
				*longp++ = data;
		}
	}
	if (size == 2) {
		for (;;) {
			shortp = (ushort *)addr;
			i = length;
			while (i-- > 0)
				*shortp++ = data;
		}
	}
	for (;;) {
		cp = (u_char *)addr;
		i = length;
		while (i-- > 0)
			*cp++ = data;
	}
}
#endif /* CONFIG_LOOPW */

/*
 * Perform a memory test. A more complete alternative test can be
 * configured using CONFIG_SYS_ALT_MEMTEST. The complete test loops until
 * interrupted by ctrl-c or by a failure of one of the sub-tests.
 */
int do_mem_mtest (cmd_tbl_t *cmdtp, int flag, int argc, char *argv[])
{
	vu_long	*addr, *start, *end;
	ulong	val;
	ulong	readback;
	int     rcode = 0;
	int iterations = 1;
	int iteration_limit;

#if defined(CONFIG_SYS_ALT_MEMTEST)
	vu_long	len;
	vu_long	offset;
	vu_long	test_offset;
	vu_long	pattern;
	vu_long	temp;
	vu_long	anti_pattern;
	vu_long	num_words;
#if defined(CONFIG_SYS_MEMTEST_SCRATCH)
	vu_long *dummy = (vu_long*)CONFIG_SYS_MEMTEST_SCRATCH;
#else
	vu_long *dummy = 0;	/* yes, this is address 0x0, not NULL */
#endif
	int	j;

	static const ulong bitpattern[] = {
		0x00000001,	/* single bit */
		0x00000003,	/* two adjacent bits */
		0x00000007,	/* three adjacent bits */
		0x0000000F,	/* four adjacent bits */
		0x00000005,	/* two non-adjacent bits */
		0x00000015,	/* three non-adjacent bits */
		0x00000055,	/* four non-adjacent bits */
		0xaaaaaaaa,	/* alternating 1/0 */
	};
#else
	ulong	incr;
	ulong	pattern;
#endif

	if (argc > 1)
		start = (ulong *)simple_strtoul(argv[1], NULL, 16);
	else
		start = (ulong *)CONFIG_SYS_MEMTEST_START;

	if (argc > 2)
		end = (ulong *)simple_strtoul(argv[2], NULL, 16);
	else
		end = (ulong *)(CONFIG_SYS_MEMTEST_END);

	if (argc > 3)
		pattern = (ulong)simple_strtoul(argv[3], NULL, 16);
	else
		pattern = 0;

	if (argc > 4)
		iteration_limit = (ulong)simple_strtoul(argv[4], NULL, 16);
	else
		iteration_limit = 0;

#if defined(CONFIG_SYS_ALT_MEMTEST)
	printf ("Testing %08x ... %08x:\n", (uint)start, (uint)end);
	PRINTF("%s:%d: start 0x%p end 0x%p\n",
		__FUNCTION__, __LINE__, start, end);

	for (;;) {
		if (ctrlc()) {
			putc ('\n');
			return 1;
		}


		if (iteration_limit && iterations > iteration_limit) {
			printf("Tested %d iteration(s) without errors.\n",
				iterations-1);
			return 0;
		}

		printf("Iteration: %6d\r", iterations);
		PRINTF("\n");
		iterations++;

		/*
		 * Data line test: write a pattern to the first
		 * location, write the 1's complement to a 'parking'
		 * address (changes the state of the data bus so a
		 * floating bus doen't give a false OK), and then
		 * read the value back. Note that we read it back
		 * into a variable because the next time we read it,
		 * it might be right (been there, tough to explain to
		 * the quality guys why it prints a failure when the
		 * "is" and "should be" are obviously the same in the
		 * error message).
		 *
		 * Rather than exhaustively testing, we test some
		 * patterns by shifting '1' bits through a field of
		 * '0's and '0' bits through a field of '1's (i.e.
		 * pattern and ~pattern).
		 */
		addr = start;
		for (j = 0; j < sizeof(bitpattern)/sizeof(bitpattern[0]); j++) {
		    val = bitpattern[j];
		    for(; val != 0; val <<= 1) {
			*addr  = val;
			*dummy  = ~val; /* clear the test data off of the bus */
			readback = *addr;
			if(readback != val) {
			     printf ("FAILURE (data line): "
				"expected %08lx, actual %08lx\n",
					  val, readback);
			}
			*addr  = ~val;
			*dummy  = val;
			readback = *addr;
			if(readback != ~val) {
			    printf ("FAILURE (data line): "
				"Is %08lx, should be %08lx\n",
					readback, ~val);
			}
		    }
		}

		/*
		 * Based on code whose Original Author and Copyright
		 * information follows: Copyright (c) 1998 by Michael
		 * Barr. This software is placed into the public
		 * domain and may be used for any purpose. However,
		 * this notice must not be changed or removed and no
		 * warranty is either expressed or implied by its
		 * publication or distribution.
		 */

		/*
		 * Address line test
		 *
		 * Description: Test the address bus wiring in a
		 *              memory region by performing a walking
		 *              1's test on the relevant bits of the
		 *              address and checking for aliasing.
		 *              This test will find single-bit
		 *              address failures such as stuck -high,
		 *              stuck-low, and shorted pins. The base
		 *              address and size of the region are
		 *              selected by the caller.
		 *
		 * Notes:	For best results, the selected base
		 *              address should have enough LSB 0's to
		 *              guarantee single address bit changes.
		 *              For example, to test a 64-Kbyte
		 *              region, select a base address on a
		 *              64-Kbyte boundary. Also, select the
		 *              region size as a power-of-two if at
		 *              all possible.
		 *
		 * Returns:     0 if the test succeeds, 1 if the test fails.
		 */
		len = ((ulong)end - (ulong)start)/sizeof(vu_long);
		pattern = (vu_long) 0xaaaaaaaa;
		anti_pattern = (vu_long) 0x55555555;

		PRINTF("%s:%d: length = 0x%.8lx\n",
			__FUNCTION__, __LINE__,
			len);
		/*
		 * Write the default pattern at each of the
		 * power-of-two offsets.
		 */
		for (offset = 1; offset < len; offset <<= 1) {
			start[offset] = pattern;
		}

		/*
		 * Check for address bits stuck high.
		 */
		test_offset = 0;
		start[test_offset] = anti_pattern;

		for (offset = 1; offset < len; offset <<= 1) {
		    temp = start[offset];
		    if (temp != pattern) {
			printf ("\nFAILURE: Address bit stuck high @ 0x%.8lx:"
				" expected 0x%.8lx, actual 0x%.8lx\n",
				(ulong)&start[offset], pattern, temp);
			return 1;
		    }
		}
		start[test_offset] = pattern;
		WATCHDOG_RESET();

		/*
		 * Check for addr bits stuck low or shorted.
		 */
		for (test_offset = 1; test_offset < len; test_offset <<= 1) {
		    start[test_offset] = anti_pattern;

		    for (offset = 1; offset < len; offset <<= 1) {
			temp = start[offset];
			if ((temp != pattern) && (offset != test_offset)) {
			    printf ("\nFAILURE: Address bit stuck low or shorted @"
				" 0x%.8lx: expected 0x%.8lx, actual 0x%.8lx\n",
				(ulong)&start[offset], pattern, temp);
			    return 1;
			}
		    }
		    start[test_offset] = pattern;
		}

		/*
		 * Description: Test the integrity of a physical
		 *		memory device by performing an
		 *		increment/decrement test over the
		 *		entire region. In the process every
		 *		storage bit in the device is tested
		 *		as a zero and a one. The base address
		 *		and the size of the region are
		 *		selected by the caller.
		 *
		 * Returns:     0 if the test succeeds, 1 if the test fails.
		 */
		num_words = ((ulong)end - (ulong)start)/sizeof(vu_long) + 1;

		/*
		 * Fill memory with a known pattern.
		 */
		for (pattern = 1, offset = 0; offset < num_words; pattern++, offset++) {
			WATCHDOG_RESET();
			start[offset] = pattern;
		}

		/*
		 * Check each location and invert it for the second pass.
		 */
		for (pattern = 1, offset = 0; offset < num_words; pattern++, offset++) {
		    WATCHDOG_RESET();
		    temp = start[offset];
		    if (temp != pattern) {
			printf ("\nFAILURE (read/write) @ 0x%.8lx:"
				" expected 0x%.8lx, actual 0x%.8lx)\n",
				(ulong)&start[offset], pattern, temp);
			return 1;
		    }

		    anti_pattern = ~pattern;
		    start[offset] = anti_pattern;
		}

		/*
		 * Check each location for the inverted pattern and zero it.
		 */
		for (pattern = 1, offset = 0; offset < num_words; pattern++, offset++) {
		    WATCHDOG_RESET();
		    anti_pattern = ~pattern;
		    temp = start[offset];
		    if (temp != anti_pattern) {
			printf ("\nFAILURE (read/write): @ 0x%.8lx:"
				" expected 0x%.8lx, actual 0x%.8lx)\n",
				(ulong)&start[offset], anti_pattern, temp);
			return 1;
		    }
		    start[offset] = 0;
		}
	}

#else /* The original, quickie test */
	incr = 1;
	for (;;) {
		if (ctrlc()) {
			putc ('\n');
			return 1;
		}

		if (iteration_limit && iterations > iteration_limit) {
			printf("Tested %d iteration(s) without errors.\n",
				iterations-1);
			return 0;
		}
		++iterations;

		printf ("\rPattern %08lX  Writing..."
			"%12s"
			"\b\b\b\b\b\b\b\b\b\b",
			pattern, "");

		for (addr=start,val=pattern; addr<end; addr++) {
			WATCHDOG_RESET();
			*addr = val;
			val  += incr;
		}

		puts ("Reading...");

		for (addr=start,val=pattern; addr<end; addr++) {
			WATCHDOG_RESET();
			readback = *addr;
			if (readback != val) {
				printf ("\nMem error @ 0x%08X: "
					"found %08lX, expected %08lX\n",
					(uint)addr, readback, val);
				rcode = 1;
			}
			val += incr;
		}

		/*
		 * Flip the pattern each time to make lots of zeros and
		 * then, the next time, lots of ones.  We decrement
		 * the "negative" patterns and increment the "positive"
		 * patterns to preserve this feature.
		 */
		if(pattern & 0x80000000) {
			pattern = -pattern;	/* complement & increment */
		}
		else {
			pattern = ~pattern;
		}
		incr = -incr;
	}
#endif
	return rcode;
}


/* Modify memory.
 *
 * Syntax:
 *	mm{.b, .w, .l} {addr}
 *	nm{.b, .w, .l} {addr}
 */
static int
mod_mem(cmd_tbl_t *cmdtp, int incrflag, int flag, int argc, char *argv[])
{
	ulong	addr, i;
	int	nbytes, size;
	extern char console_buffer[];

	if (argc != 2) {
		cmd_usage(cmdtp);
		return 1;
	}

#ifdef CONFIG_BOOT_RETRY_TIME
	reset_cmd_timeout();	/* got a good command to get here */
#endif
	/* We use the last specified parameters, unless new ones are
	 * entered.
	 */
	addr = mm_last_addr;
	size = mm_last_size;

	if ((flag & CMD_FLAG_REPEAT) == 0) {
		/* New command specified.  Check for a size specification.
		 * Defaults to long if no or incorrect specification.
		 */
		if ((size = cmd_get_data_size(argv[0], 4)) < 0)
			return 1;

		/* Address is specified since argc > 1
		*/
		addr = simple_strtoul(argv[1], NULL, 16);
		addr += base_address;
	}

#ifdef CONFIG_HAS_DATAFLASH
	if (addr_dataflash(addr)){
		puts ("Can't modify DataFlash in place. Use cp instead.\n\r");
		return 0;
	}
#endif

#ifdef CONFIG_BLACKFIN
	if (addr_bfin_on_chip_mem(addr)) {
		puts ("Can't modify L1 instruction in place. Use cp instead.\n\r");
		return 0;
	}
#endif

	/* Print the address, followed by value.  Then accept input for
	 * the next value.  A non-converted value exits.
	 */
	do {
		printf("%08lx:", addr);
		if (size == 4)
			printf(" %08x", *((uint   *)addr));
		else if (size == 2)
			printf(" %04x", *((ushort *)addr));
		else
			printf(" %02x", *((u_char *)addr));

		nbytes = readline (" ? ");
		if (nbytes == 0 || (nbytes == 1 && console_buffer[0] == '-')) {
			/* <CR> pressed as only input, don't modify current
			 * location and move to next. "-" pressed will go back.
			 */
			if (incrflag)
				addr += nbytes ? -size : size;
			nbytes = 1;
#ifdef CONFIG_BOOT_RETRY_TIME
			reset_cmd_timeout(); /* good enough to not time out */
#endif
		}
#ifdef CONFIG_BOOT_RETRY_TIME
		else if (nbytes == -2) {
			break;	/* timed out, exit the command	*/
		}
#endif
		else {
			char *endp;
			i = simple_strtoul(console_buffer, &endp, 16);
			nbytes = endp - console_buffer;
			if (nbytes) {
#ifdef CONFIG_BOOT_RETRY_TIME
				/* good enough to not time out
				 */
				reset_cmd_timeout();
#endif
				if (size == 4)
					*((uint   *)addr) = i;
				else if (size == 2)
					*((ushort *)addr) = i;
				else
					*((u_char *)addr) = i;
				if (incrflag)
					addr += size;
			}
		}
	} while (nbytes);

	mm_last_addr = addr;
	mm_last_size = size;
	return 0;
}

#ifndef CONFIG_CRC32_VERIFY

int do_mem_crc (cmd_tbl_t *cmdtp, int flag, int argc, char *argv[])
{
	ulong addr, length;
	ulong crc;
	ulong *ptr;

	if (argc < 3) {
		cmd_usage(cmdtp);
		return 1;
	}

	addr = simple_strtoul (argv[1], NULL, 16);
	addr += base_address;

	length = simple_strtoul (argv[2], NULL, 16);

	crc = crc32 (0, (const uchar *) addr, length);

	printf ("CRC32 for %08lx ... %08lx ==> %08lx\n",
			addr, addr + length - 1, crc);

	if (argc > 3) {
		ptr = (ulong *) simple_strtoul (argv[3], NULL, 16);
		*ptr = crc;
	}

	return 0;
}

#else	/* CONFIG_CRC32_VERIFY */

int do_mem_crc (cmd_tbl_t *cmdtp, int flag, int argc, char *argv[])
{
	ulong addr, length;
	ulong crc;
	ulong *ptr;
	ulong vcrc;
	int verify;
	int ac;
	char **av;

	if (argc < 3) {
  usage:
		cmd_usage(cmdtp);
		return 1;
	}

	av = argv + 1;
	ac = argc - 1;
	if (strcmp(*av, "-v") == 0) {
		verify = 1;
		av++;
		ac--;
		if (ac < 3)
			goto usage;
	} else
		verify = 0;

	addr = simple_strtoul(*av++, NULL, 16);
	addr += base_address;
	length = simple_strtoul(*av++, NULL, 16);

	crc = crc32(0, (const uchar *) addr, length);

	if (!verify) {
		printf ("CRC32 for %08lx ... %08lx ==> %08lx\n",
				addr, addr + length - 1, crc);
		if (ac > 2) {
			ptr = (ulong *) simple_strtoul (*av++, NULL, 16);
			*ptr = crc;
		}
	} else {
		vcrc = simple_strtoul(*av++, NULL, 16);
		if (vcrc != crc) {
			printf ("CRC32 for %08lx ... %08lx ==> %08lx != %08lx ** ERROR **\n",
					addr, addr + length - 1, crc, vcrc);
			return 1;
		}
	}

	return 0;

}
#endif	/* CONFIG_CRC32_VERIFY */


#ifdef CONFIG_CMD_UNZIP
int  gunzip (void *, int, unsigned char *, unsigned long *);

int do_unzip ( cmd_tbl_t *cmdtp, int flag, int argc, char *argv[])
{
	unsigned long src, dst;
	unsigned long src_len = ~0UL, dst_len = ~0UL;

	switch (argc) {
		case 4:
			dst_len = simple_strtoul(argv[3], NULL, 16);
			/* fall through */
		case 3:
			src = simple_strtoul(argv[1], NULL, 16);
			dst = simple_strtoul(argv[2], NULL, 16);
			break;
		default:
			cmd_usage(cmdtp);
			return 1;
	}

	return !!gunzip((void *) dst, dst_len, (void *) src, &src_len);
}
#endif /* CONFIG_CMD_UNZIP */


/**************************************************/
U_BOOT_CMD(
	md,	3,	1,	do_mem_md,
	"memory display",
	"[.b, .w, .l] address [# of objects]"
);


U_BOOT_CMD(
	mm,	2,	1,	do_mem_mm,
	"memory modify (auto-incrementing address)",
	"[.b, .w, .l] address"
);


U_BOOT_CMD(
	nm,	2,	1,	do_mem_nm,
	"memory modify (constant address)",
	"[.b, .w, .l] address"
);

U_BOOT_CMD(
	mw,	4,	1,	do_mem_mw,
	"memory write (fill)",
	"[.b, .w, .l] address value [count]"
);

U_BOOT_CMD(
	cp,	4,	1,	do_mem_cp,
	"memory copy",
	"[.b, .w, .l] source target count"
);

U_BOOT_CMD(
	cmp,	4,	1,	do_mem_cmp,
	"memory compare",
	"[.b, .w, .l] addr1 addr2 count"
);

#ifndef CONFIG_CRC32_VERIFY

U_BOOT_CMD(
	crc32,	4,	1,	do_mem_crc,
	"checksum calculation",
	"address count [addr]\n    - compute CRC32 checksum [save at addr]"
);

#else	/* CONFIG_CRC32_VERIFY */

U_BOOT_CMD(
	crc32,	5,	1,	do_mem_crc,
	"checksum calculation",
	"address count [addr]\n    - compute CRC32 checksum [save at addr]\n"
	"-v address count crc\n    - verify crc of memory area"
);

#endif	/* CONFIG_CRC32_VERIFY */

U_BOOT_CMD(
	base,	2,	1,	do_mem_base,
	"print or set address offset",
	"\n    - print address offset for memory commands\n"
	"base off\n    - set address offset for memory commands to 'off'"
);

U_BOOT_CMD(
	loop,	3,	1,	do_mem_loop,
	"infinite loop on address range",
	"[.b, .w, .l] address number_of_objects"
);

#ifdef CONFIG_LOOPW
U_BOOT_CMD(
	loopw,	4,	1,	do_mem_loopw,
	"infinite write loop on address range",
	"[.b, .w, .l] address number_of_objects data_to_write"
);
#endif /* CONFIG_LOOPW */

U_BOOT_CMD(
	mtest,	5,	1,	do_mem_mtest,
	"simple RAM read/write test",
	"[start [end [pattern [iterations]]]]"
);

#ifdef CONFIG_MX_CYCLIC
U_BOOT_CMD(
	mdc,	4,	1,	do_mem_mdc,
	"memory display cyclic",
	"[.b, .w, .l] address count delay(ms)"
);

U_BOOT_CMD(
	mwc,	4,	1,	do_mem_mwc,
	"memory write cyclic",
	"[.b, .w, .l] address value delay(ms)"
);
#endif /* CONFIG_MX_CYCLIC */

#ifdef CONFIG_CMD_UNZIP
U_BOOT_CMD(
	unzip,	4,	1,	do_unzip,
	"unzip a memory region",
	"srcaddr dstaddr [dstsize]"
);
#endif /* CONFIG_CMD_UNZIP */
