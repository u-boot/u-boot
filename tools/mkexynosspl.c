/*
 * Copyright (C) 2012 Samsung Electronics
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
#include <fcntl.h>
#include <errno.h>
#include <string.h>
#include <sys/stat.h>
#include <compiler.h>

#define CHECKSUM_OFFSET		(14*1024-4)
#define BUFSIZE			(14*1024)
#define FILE_PERM		(S_IRUSR | S_IWUSR | S_IRGRP \
				| S_IWGRP | S_IROTH | S_IWOTH)
/*
* Requirement:
* IROM code reads first 14K bytes from boot device.
* It then calculates the checksum of 14K-4 bytes and compare with data at
* 14K-4 offset.
*
* This function takes two filenames:
* IN  "u-boot-spl.bin" and
* OUT "$(BOARD)-spl.bin as filenames.
* It reads the "u-boot-spl.bin" in 16K buffer.
* It calculates checksum of 14K-4 Bytes and stores at 14K-4 offset in buffer.
* It writes the buffer to "$(BOARD)-spl.bin" file.
*/

int main(int argc, char **argv)
{
	unsigned char buffer[BUFSIZE];
	int i, ifd, ofd;
	uint32_t checksum = 0;
	off_t	len;
	ssize_t count;
	struct stat stat;

	if (argc != 3) {
		fprintf(stderr, "Usage: %s <infile> <outfile>\n", argv[0]);
		exit(EXIT_FAILURE);
	}

	ifd = open(argv[1], O_RDONLY);
	if (ifd < 0) {
		fprintf(stderr, "%s: Can't open %s: %s\n",
			argv[0], argv[1], strerror(errno));
		exit(EXIT_FAILURE);
	}

	ofd = open(argv[2], O_WRONLY | O_CREAT | O_TRUNC, FILE_PERM);
	if (ifd < 0) {
		fprintf(stderr, "%s: Can't open %s: %s\n",
			argv[0], argv[2], strerror(errno));
		close(ifd);
		exit(EXIT_FAILURE);
	}

	if (fstat(ifd, &stat)) {
		fprintf(stderr, "%s: Unable to get size of %s: %s\n",
			argv[0], argv[1], strerror(errno));
		close(ifd);
		close(ofd);
		exit(EXIT_FAILURE);
	}

	len = stat.st_size;

	count = (len < CHECKSUM_OFFSET) ? len : CHECKSUM_OFFSET;

	if (read(ifd, buffer, count) != count) {
		fprintf(stderr, "%s: Can't read %s: %s\n",
			argv[0], argv[1], strerror(errno));

		close(ifd);
		close(ofd);

		exit(EXIT_FAILURE);
	}

	for (i = 0, checksum = 0; i < CHECKSUM_OFFSET; i++)
		checksum += buffer[i];

	checksum = cpu_to_le32(checksum);

	memcpy(&buffer[CHECKSUM_OFFSET], &checksum, sizeof(checksum));

	if (write(ofd, buffer, BUFSIZE) != BUFSIZE) {
		fprintf(stderr, "%s: Can't write %s: %s\n",
			argv[0], argv[2], strerror(errno));

		close(ifd);
		close(ofd);

		exit(EXIT_FAILURE);
	}

	close(ifd);
	close(ofd);

	return EXIT_SUCCESS;
}
