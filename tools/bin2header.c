/* bin2header.c - program to convert binary file into a C structure
 * definition to be included in a header file.
 *
 * (C) Copyright 2008 by Harald Welte <laforge@openmoko.org>
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

#include <stdlib.h>
#include <stdio.h>

int main(int argc, char **argv)
{
	if (argc < 2) {
		fprintf(stderr, "%s needs one argument: the structure name\n",
			argv[0]);
		exit(1);
	}

	printf("/* bin2header output - automatically generated */\n");
	printf("unsigned char %s[] = {\n", argv[1]);

	while (1) {
		int i, nread;
		unsigned char buf[10];
		nread = read(0, buf, sizeof(buf));
		if (nread <= 0)
			break;

		printf("\t");
		for (i = 0; i < nread - 1; i++)
			printf("0x%02x, ", buf[i]);

		printf("0x%02x,\n", buf[nread-1]);
	}

	printf("};\n");

	exit(0);
}
