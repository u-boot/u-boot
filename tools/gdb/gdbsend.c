/*
 * (C) Copyright 2000
 * Murray Jensen <Murray.Jensen@csiro.au>
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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include "serial.h"
#include "error.h"
#include "remote.h"

char *serialdev = "/dev/term/b";
speed_t speed = B230400;
int verbose = 0, docont = 0;
unsigned long addr = 0x10000UL;

int
main(int ac, char **av)
{
    int c, sfd, ifd;
    char *ifn, *image;
    struct stat ist;

    if ((pname = strrchr(av[0], '/')) == NULL)
	pname = av[0];
    else
	pname++;

    while ((c = getopt(ac, av, "a:b:cp:v")) != EOF)
	switch (c) {

	case 'a': {
	    char *ep;

	    addr = strtol(optarg, &ep, 0);
	    if (ep == optarg || *ep != '\0')
		Error("can't decode address specified in -a option");
	    break;
	}

	case 'b':
	    if ((speed = cvtspeed(optarg)) == B0)
		Error("can't decode baud rate specified in -b option");
	    break;

	case 'c':
	    docont = 1;
	    break;

	case 'p':
	    serialdev = optarg;
	    break;

	case 'v':
	    verbose = 1;
	    break;

	default:
	usage:
	    fprintf(stderr,
		"Usage: %s [-a addr] [-b bps] [-c] [-p dev] [-v] imagefile\n",
		pname);
	    exit(1);
	}

    if (optind != ac - 1)
	goto usage;
    ifn = av[optind++];

    if (verbose)
	fprintf(stderr, "Opening file and reading image...\n");

    if ((ifd = open(ifn, O_RDONLY)) < 0)
	Perror("can't open kernel image file '%s'", ifn);

    if (fstat(ifd, &ist) < 0)
	Perror("fstat '%s' failed", ifn);

    if ((image = (char *)malloc(ist.st_size)) == NULL)
	Perror("can't allocate %ld bytes for image", ist.st_size);

    if ((c = read(ifd, image, ist.st_size)) < 0)
	Perror("read of %d bytes from '%s' failed", ist.st_size, ifn);

    if (c != ist.st_size)
	Error("read of %ld bytes from '%s' failed (%d)", ist.st_size, ifn, c);

    if (close(ifd) < 0)
	Perror("close of '%s' failed", ifn);

    if (verbose)
	fprintf(stderr, "Opening serial port and sending image...\n");

    if ((sfd = serialopen(serialdev, speed)) < 0)
	Perror("open of serial device '%s' failed", serialdev);

    remote_desc = sfd;
    remote_reset();
    remote_write_bytes(addr, image, ist.st_size);

    if (docont) {
	if (verbose)
	    fprintf(stderr, "[continue]");
	remote_continue();
    }

    if (serialclose(sfd) < 0)
	Perror("close of serial device '%s' failed", serialdev);

    if (verbose)
	fprintf(stderr, "Done.\n");

    return (0);
}
