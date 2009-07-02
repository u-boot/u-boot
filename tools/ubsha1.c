/*
 * (C) Copyright 2007
 * Heiko Schocher, DENX Software Engineering, <hs@denx.de>
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

#include "os_support.h"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <string.h>
#include <sys/stat.h>
#include "sha1.h"

#ifndef __ASSEMBLY__
#define	__ASSEMBLY__		/* Dirty trick to get only #defines	*/
#endif
#include <config.h>
#undef	__ASSEMBLY__

extern void sha1_csum (unsigned char *input, int ilen, unsigned char output[20]);

int main (int argc, char **argv)
{
	unsigned char output[20];
	int i, len;

	char	*imagefile;
	char	*cmdname = *argv;
	unsigned char	*ptr;
	unsigned char	*data;
	struct stat sbuf;
	unsigned char	*ptroff;
	int	ifd;
	int	off;

	if (argc > 1) {
		imagefile = argv[1];
		ifd = open (imagefile, O_RDWR|O_BINARY);
		if (ifd < 0) {
			fprintf (stderr, "%s: Can't open %s: %s\n",
				cmdname, imagefile, strerror(errno));
			exit (EXIT_FAILURE);
		}
		if (fstat (ifd, &sbuf) < 0) {
			fprintf (stderr, "%s: Can't stat %s: %s\n",
				cmdname, imagefile, strerror(errno));
			exit (EXIT_FAILURE);
		}
		len = sbuf.st_size;
		ptr = (unsigned char *)mmap(0, len,
				    PROT_READ, MAP_SHARED, ifd, 0);
		if (ptr == (unsigned char *)MAP_FAILED) {
			fprintf (stderr, "%s: Can't read %s: %s\n",
				cmdname, imagefile, strerror(errno));
			exit (EXIT_FAILURE);
		}

		/* create a copy, so we can blank out the sha1 sum */
		data = malloc (len);
		memcpy (data, ptr, len);
		off = SHA1_SUM_POS;
		ptroff = &data[len +  off];
		for (i = 0; i < SHA1_SUM_LEN; i++) {
			ptroff[i] = 0;
		}

		sha1_csum ((unsigned char *) data, len, (unsigned char *)output);

		printf ("U-Boot sum:\n");
	        for (i = 0; i < 20 ; i++) {
	            printf ("%02X ", output[i]);
	        }
	        printf ("\n");
		/* overwrite the sum in the bin file, with the actual */
		lseek (ifd, SHA1_SUM_POS, SEEK_END);
		if (write (ifd, output, SHA1_SUM_LEN) != SHA1_SUM_LEN) {
			fprintf (stderr, "%s: Can't write %s: %s\n",
				cmdname, imagefile, strerror(errno));
			exit (EXIT_FAILURE);
		}

		free (data);
		(void) munmap((void *)ptr, len);
		(void) close (ifd);
	}

	return EXIT_SUCCESS;
}
