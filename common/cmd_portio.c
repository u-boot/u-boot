/*
 * (C) Copyright 2003
 * Marc Singer, elf@buici.com
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

/*
 * Port I/O Functions
 *
 * Copied from FADS ROM, Dan Malek (dmalek@jlc.net)
 */

#include <common.h>
#include <command.h>

/* Display values from last command.
 * Memory modify remembered values are different from display memory.
 */
static uint in_last_addr, in_last_size;
static uint out_last_addr, out_last_size, out_last_value;


int do_portio_out (cmd_tbl_t * cmdtp, int flag, int argc, char * const argv[])
{
	uint addr = out_last_addr;
	uint size = out_last_size;
	uint value = out_last_value;

	if (argc != 3)
		return CMD_RET_USAGE;

	if ((flag & CMD_FLAG_REPEAT) == 0) {
		/*
		 * New command specified.  Check for a size specification.
		 * Defaults to long if no or incorrect specification.
		 */
		size = cmd_get_data_size (argv[0], 1);
		addr = simple_strtoul (argv[1], NULL, 16);
		value = simple_strtoul (argv[2], NULL, 16);
	}
#if defined (CONFIG_X86)

	{
		unsigned short port = addr;

		switch (size) {
		default:
		case 1:
		    {
			unsigned char ch = value;
			__asm__ volatile ("out %0, %%dx"::"a" (ch), "d" (port));
		    }
			break;
		case 2:
		    {
			unsigned short w = value;
			__asm__ volatile ("out %0, %%dx"::"a" (w), "d" (port));
		    }
			break;
		case 4:
			__asm__ volatile ("out %0, %%dx"::"a" (value), "d" (port));

			break;
		}
	}

#endif							/* CONFIG_X86 */

	out_last_addr = addr;
	out_last_size = size;
	out_last_value = value;

	return 0;
}

U_BOOT_CMD(
	out,	3,	1,	do_portio_out,
	"write datum to IO port",
	"[.b, .w, .l] port value\n    - output to IO port"
);

int do_portio_in (cmd_tbl_t * cmdtp, int flag, int argc, char * const argv[])
{
	uint addr = in_last_addr;
	uint size = in_last_size;

	if (argc != 2)
		return CMD_RET_USAGE;

	if ((flag & CMD_FLAG_REPEAT) == 0) {
		/*
		 * New command specified.  Check for a size specification.
		 * Defaults to long if no or incorrect specification.
		 */
		size = cmd_get_data_size (argv[0], 1);
		addr = simple_strtoul (argv[1], NULL, 16);
	}
#if defined (CONFIG_X86)

	{
		unsigned short port = addr;

		switch (size) {
		default:
		case 1:
		    {
			unsigned char ch;
			__asm__ volatile ("in %%dx, %0":"=a" (ch):"d" (port));

			printf (" %02x\n", ch);
		    }
			break;
		case 2:
		    {
			unsigned short w;
			__asm__ volatile ("in %%dx, %0":"=a" (w):"d" (port));

			printf (" %04x\n", w);
		    }
			break;
		case 4:
		    {
			unsigned long l;
			__asm__ volatile ("in %%dx, %0":"=a" (l):"d" (port));

			printf (" %08lx\n", l);
		    }
			break;
		}
	}
#endif	/* CONFIG_X86 */

	in_last_addr = addr;
	in_last_size = size;

	return 0;
}

U_BOOT_CMD(
	in,	2,	1,	do_portio_in,
	"read data from an IO port",
	"[.b, .w, .l] port\n"
	"    - read datum from IO port"
);
