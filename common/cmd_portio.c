/*
 * (C) Copyright 2003
 * Marc Singer, elf@buici.com
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
 * Port I/O Functions
 *
 * Copied from FADS ROM, Dan Malek (dmalek@jlc.net)
 */

#include <common.h>
#include <command.h>

extern int cmd_get_data_size (char *arg, int default_size);

/* Display values from last command.
 * Memory modify remembered values are different from display memory.
 */
static uint in_last_addr, in_last_size;
static uint out_last_addr, out_last_size, out_last_value;


int do_portio_out (cmd_tbl_t * cmdtp, int flag, int argc, char *argv[])
{
	uint addr = out_last_addr;
	uint size = out_last_size;
	uint value = out_last_value;

	if (argc != 3) {
		printf ("Usage:\n%s\n", cmdtp->usage);
		return 1;
	}

	if ((flag & CMD_FLAG_REPEAT) == 0) {
		/* New command specified.  Check for a size specification.
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
	"out     - write datum to IO port\n",
	"[.b, .w, .l] port value\n    - output to IO port\n"
);

int do_portio_in (cmd_tbl_t * cmdtp, int flag, int argc, char *argv[])
{
	uint addr = in_last_addr;
	uint size = in_last_size;

	if (argc != 2) {
		printf ("Usage:\n%s\n", cmdtp->usage);
		return 1;
	}

	if ((flag & CMD_FLAG_REPEAT) == 0) {
		/* New command specified.  Check for a size specification.
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
	"in      - read data from an IO port\n",
	"[.b, .w, .l] port\n"
	"    - read datum from IO port\n"
);
