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
#include <unistd.h>
#include "serial.h"
#include "error.h"
#include "remote.h"

char *serialdev = "/dev/term/b";
speed_t speed = B230400;
int verbose = 0;

int
main(int ac, char **av)
{
    int c, sfd;

    if ((pname = strrchr(av[0], '/')) == NULL)
	pname = av[0];
    else
	pname++;

    while ((c = getopt(ac, av, "b:p:v")) != EOF)
	switch (c) {

	case 'b':
	    if ((speed = cvtspeed(optarg)) == B0)
		Error("can't decode baud rate specified in -b option");
	    break;

	case 'p':
	    serialdev = optarg;
	    break;

	case 'v':
	    verbose = 1;
	    break;

	default:
	usage:
	    fprintf(stderr, "Usage: %s [-b bps] [-p dev] [-v]\n", pname);
	    exit(1);
	}
    if (optind != ac)
	goto usage;

    if (verbose)
	fprintf(stderr, "Opening serial port and sending continue...\n");

    if ((sfd = serialopen(serialdev, speed)) < 0)
	Perror("open of serial device '%s' failed", serialdev);

    remote_desc = sfd;
    remote_reset();
    remote_continue();

    if (serialclose(sfd) < 0)
	Perror("close of serial device '%s' failed", serialdev);

    if (verbose)
	fprintf(stderr, "Done.\n");

    return (0);
}
