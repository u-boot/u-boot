/*
 * (C) Copyright 2001
 * Murray Jensen, CSIRO Manufacturing Science and Technology,
 * <Murray.Jensen@cmst.csiro.au>
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

#include <common.h>
#include <net.h>

/* imports from common/main.c */
extern char console_buffer[CFG_CBSIZE];

int
fetch_and_parse(bd_t *bd, char *fn, ulong addr, int (*cback)(uchar *, uchar *))
{
    char *ethaddr;
    uchar *fp, *efp;

    while ((ethaddr = getenv("ethaddr")) == NULL || *ethaddr == '\0') {

	puts("*** Ethernet address is not set\n");

	for (;;) {
	    int n;

	    n = readline("Enter board ethernet address: ");

	    if (n < 0) {
		puts("\n");
		return (0);
	    }

	    if (n == 0)
		continue;

	    if (n == 17) {
		int i;
		char *p, *q;
		uchar ea[6];

		/* see if it looks like an ethernet address */

		p = console_buffer;

		for (i = 0; i < 6; i++) {
		    char term = (i == 5 ? '\0' : ':');

		    ea[i] = simple_strtol(p, &q, 16);

		    if ((q - p) != 2 || *q++ != term)
			break;

		    p = q;
		}

		if (i == 6) {
		    /* it looks ok - set it */
		    printf("Setting ethernet address to %s\n", console_buffer);
		    setenv("ethaddr", console_buffer);

		    puts("Remember to do a 'saveenv' to make it permanent\n");
		    break;
		}
	    }

	    printf("Invalid ethernet address (%s) - please re-enter\n",
		console_buffer);
	}
    }

    copy_filename(BootFile, fn, sizeof (BootFile));
    load_addr = addr;

    if (NetLoop(TFTP) <= 0) {
	printf("tftp transfer of file '%s' failed\n", fn);
	return (0);
    }

    if (NetBootFileXferSize == 0) {
	printf("can't determine size of file '%s'\n", fn);
	return (0);
    }

    fp = (uchar *)load_addr;
    efp = fp + NetBootFileXferSize;

    do {
	uchar *name, *value;

	if (*fp == '#' || *fp == '\n') {
	    while (fp < efp && *fp++ != '\n')
		;
	    continue;
	}

	name = fp;

	while (fp < efp && *fp != '=')
	    if (*fp++ == '\n')
		name = fp;

	if (fp >= efp)
	    break;

	*fp++ = '\0';

	value = fp;

	while (fp < efp && *fp != '\n')
	    fp++;

	/* ok if we go off the end here */

	if (fp[-1] == '\r')
	    fp[-1] = '\0';
	*fp++ = '\0';

	if ((*cback)(name, value) == 0)
	    return (0);

    } while (fp < efp);

    return (1);
}
