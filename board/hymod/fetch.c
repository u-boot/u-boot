/*
 * (C) Copyright 2001
 * Murray Jensen, CSIRO-MIT, <Murray.Jensen@csiro.au>
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <common.h>
#include <net.h>

/* imports from input.c */
extern int hymod_get_ethaddr (void);

int
fetch_and_parse (char *fn, ulong addr, int (*cback)(uchar *, uchar *))
{
	char *ethaddr;
	uchar *fp, *efp;
	int rc, count = 0;

	while ((ethaddr = getenv ("ethaddr")) == NULL || *ethaddr == '\0') {

		printf ("*** Ethernet address is%s not set\n",
			count == 0 ? "" : " STILL");

		if ((rc = hymod_get_ethaddr ()) < 0) {
			if (rc == -1)
				puts ("\n*** interrupted!");
			else
				puts ("\n*** timeout!");
			printf (" - fetch of '%s' aborted\n", fn);
			return (0);
		}

		count++;
	}

	copy_filename (BootFile, fn, sizeof (BootFile));
	load_addr = addr;
	NetBootFileXferSize = 0;

	if (NetLoop(TFTPGET) == 0) {
		printf ("tftp transfer of file '%s' failed\n", fn);
		return (0);
	}

	if (NetBootFileXferSize == 0) {
		printf ("can't determine size of file '%s'\n", fn);
		return (0);
	}

	fp = (uchar *)load_addr;
	efp = fp + NetBootFileXferSize;

	do {
		uchar *name, *value;

		if (*fp == '#' || *fp == '\n') {
			/* skip this line */
			while (fp < efp && *fp++ != '\n')
				;
			continue;
		}

		name = fp;

		while (fp < efp && *fp != '=' && *fp != '\n')
			fp++;
		if (fp >= efp)
			break;
		if (*fp == '\n') {
			fp++;
			continue;
		}
		*fp++ = '\0';

		value = fp;

		while (fp < efp && *fp != '\n')
			fp++;
		if (fp[-1] == '\r')
			fp[-1] = '\0';
		*fp++ = '\0';	/* ok if we go off the end here */

		if ((*cback)(name, value) == 0)
			return (0);

	} while (fp < efp);

	return (1);
}
