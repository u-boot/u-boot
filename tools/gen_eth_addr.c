/*
 * (C) Copyright 2001
 * Murray Jensen <Murray.Jensen@cmst.csiro.au>
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
#include <unistd.h>
#include <time.h>

int
main(int argc, char *argv[])
{
    unsigned long ethaddr_low, ethaddr_high;

    srandom(time(0) | getpid());

    /*
     * setting the 2nd LSB in the most significant byte of
     * the address makes it a locally administered ethernet
     * address
     */
    ethaddr_high = (random() & 0xfeff) | 0x0200;
    ethaddr_low = random();

    printf("%02lx:%02lx:%02lx:%02lx:%02lx:%02lx\n",
	ethaddr_high >> 8, ethaddr_high & 0xff,
	ethaddr_low >> 24, (ethaddr_low >> 16) & 0xff,
	(ethaddr_low >> 8) & 0xff, ethaddr_low & 0xff);

    return (0);
}
