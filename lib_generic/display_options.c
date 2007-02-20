/*
 * (C) Copyright 2000-2002
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

#include <config.h>
#include <common.h>
#include <linux/ctype.h>
#include <asm/io.h>

int display_options (void)
{
	extern char version_string[];

#if defined(BUILD_TAG)
	printf ("\n\n%s, Build: %s\n\n", version_string, BUILD_TAG);
#else
	printf ("\n\n%s\n\n", version_string);
#endif
	return 0;
}

/*
 * print sizes as "xxx kB", "xxx.y kB", "xxx MB" or "xxx.y MB" as needed;
 * allow for optional trailing string (like "\n")
 */
void print_size (ulong size, const char *s)
{
	ulong m, n;
	ulong d = 1 << 20;		/* 1 MB */
	char  c = 'M';

	if (size < d) {			/* print in kB */
		c = 'k';
		d = 1 << 10;
	}

	n = size / d;

	m = (10 * (size - (n * d)) + (d / 2) ) / d;

	if (m >= 10) {
		m -= 10;
		n += 1;
	}

	printf ("%2ld", n);
	if (m) {
		printf (".%ld", m);
	}
	printf (" %cB%s", c, s);
}

/*
 * Print data buffer in hex and ascii form to the terminal.
 *
 * data reads are buffered so that each memory address is only read once.
 * Useful when displaying the contents of volatile registers.
 *
 * parameters:
 *    addr: Starting address to display at start of line
 *    data: pointer to data buffer
 *    width: data value width.  May be 1, 2, or 4.
 *    count: number of values to display
 *    linelen: Number of values to print per line; specify 0 for default length
 */
#define MAX_LINE_LENGTH_BYTES (64)
#define DEFAULT_LINE_LENGTH_BYTES (16)
int print_buffer (ulong addr, void* data, uint width, uint count, uint linelen)
{
	uint8_t linebuf[MAX_LINE_LENGTH_BYTES];
	uint32_t *uip = (void*)linebuf;
	uint16_t *usp = (void*)linebuf;
	uint8_t *ucp = (void*)linebuf;
	int i;

	if (linelen*width > MAX_LINE_LENGTH_BYTES)
		linelen = MAX_LINE_LENGTH_BYTES / width;
	if (linelen < 1)
		linelen = DEFAULT_LINE_LENGTH_BYTES / width;

	while (count) {
		printf("%08lx:", addr);

		/* check for overflow condition */
		if (count < linelen)
			linelen = count;

		/* Copy from memory into linebuf and print hex values */
		for (i = 0; i < linelen; i++) {
			if (width == 4) {
				uip[i] = *(volatile uint32_t *)data;
				printf(" %08x", uip[i]);
			} else if (width == 2) {
				usp[i] = *(volatile uint16_t *)data;
				printf(" %04x", usp[i]);
			} else {
				ucp[i] = *(volatile uint8_t *)data;
				printf(" %02x", ucp[i]);
			}
			data += width;
		}

		/* Print data in ASCII characters */
		puts("    ");
		for (i = 0; i < linelen * width; i++)
			putc(isprint(ucp[i]) && (ucp[i] < 0x80) ? ucp[i] : '.');
		putc ('\n');

		/* update references */
		addr += linelen * width;
		count -= linelen;

		if (ctrlc())
			return -1;
	}

	return 0;
}
