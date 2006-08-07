/*
 * (C) Copyright 2005
 * 2N Telekomunikace, Ladislav Michl <michl@2n.cz>
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
#include <stdint.h>
#include <fcntl.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include "crcek.h"

extern unsigned long crc32(unsigned long, const unsigned char *, unsigned int);

uint32_t data[LOADER_SIZE/4 + 3];

int doit(char *path, unsigned version)
{
	uint32_t *p;
	ssize_t size;
	int fd;

	fd = open(path, O_RDONLY);
	if (fd == -1) {
		perror("Error opening file");
		return EXIT_FAILURE;
	}
	p = data + 2;
	size = read(fd, p, LOADER_SIZE + 4);
	if (size == -1) {
		perror("Error reading file");
		return EXIT_FAILURE;
	}
	if (size > LOADER_SIZE) {
		fprintf(stderr, "File too large\n");
		return EXIT_FAILURE;
	}
	size = (((size - 1) >> 2) + 1) << 2;
	data[0] = size + 4;	/* add size of version field */
	data[1] = version;
	data[(size >> 2) + 2] = crc32(0, (unsigned char *)(data + 1), data[0]);
	close(fd);

	if (write(STDOUT_FILENO, data, size + 3*4) == -1) {
		perror("Error writing file");
		return EXIT_FAILURE;
	}

	return EXIT_SUCCESS;
}

int main(int argc, char **argv)
{
	if (argc == 2) {
		return doit(argv[1], 0);
	} else if ((argc == 4) && (strcmp(argv[1], "-v") == 0)) {
		char *endptr, *nptr = argv[2];
		unsigned ver = strtoul(nptr, &endptr, 0);
		if (*nptr != '\0' && *endptr == '\0')
			return doit(argv[3], ver);
	}
	fprintf(stderr, "Usage: crcit [-v version] <image>\n");

	return EXIT_FAILURE;
}
