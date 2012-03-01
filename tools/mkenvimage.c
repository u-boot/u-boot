/*
 * (C) Copyright 2011 Free Electrons
 * David Wagner <david.wagner@free-electrons.com>
 *
 * Inspired from envcrc.c:
 * (C) Copyright 2001
 * Paolo Scaffardi, AIRVENT SAM s.p.a - RIMINI(ITALY), arsenio@tin.it
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

/* We want the GNU version of basename() */
#define _GNU_SOURCE

#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <unistd.h>
#include <compiler.h>
#include <sys/types.h>
#include <sys/stat.h>

#include <u-boot/crc.h>
#include <version.h>

#define CRC_SIZE sizeof(uint32_t)

static void usage(const char *exec_name)
{
	fprintf(stderr, "%s [-h] [-r] [-b] [-p <byte>] "
	       "-s <environment partition size> -o <output> <input file>\n"
	       "\n"
	       "This tool takes a key=value input file (same as would a "
	       "`printenv' show) and generates the corresponding environment "
	       "image, ready to be flashed.\n"
	       "\n"
	       "\tThe input file is in format:\n"
	       "\t\tkey1=value1\n"
	       "\t\tkey2=value2\n"
	       "\t\t...\n"
	       "\t-r : the environment has multiple copies in flash\n"
	       "\t-b : the target is big endian (default is little endian)\n"
	       "\t-p <byte> : fill the image with <byte> bytes instead of "
	       "0xff bytes\n"
	       "\t-V : print version information and exit\n"
	       "\n"
	       "If the input file is \"-\", data is read from standard input\n",
	       exec_name);
}

int main(int argc, char **argv)
{
	uint32_t crc, targetendian_crc;
	const char *txt_filename = NULL, *bin_filename = NULL;
	int txt_fd, bin_fd;
	unsigned char *dataptr, *envptr;
	unsigned char *filebuf = NULL;
	unsigned int filesize = 0, envsize = 0, datasize = 0;
	int bigendian = 0;
	int redundant = 0;
	unsigned char padbyte = 0xff;

	int option;
	int ret = EXIT_SUCCESS;

	struct stat txt_file_stat;

	int fp, ep;
	const char *prg;

	prg = basename(argv[0]);

	/* Turn off getopt()'s internal error message */
	opterr = 0;

	/* Parse the cmdline */
	while ((option = getopt(argc, argv, ":s:o:rbp:hV")) != -1) {
		switch (option) {
		case 's':
			datasize = strtol(optarg, NULL, 0);
			break;
		case 'o':
			bin_filename = strdup(optarg);
			if (!bin_filename) {
				fprintf(stderr, "Can't strdup() the output "
						"filename\n");
				return EXIT_FAILURE;
			}
			break;
		case 'r':
			redundant = 1;
			break;
		case 'b':
			bigendian = 1;
			break;
		case 'p':
			padbyte = strtol(optarg, NULL, 0);
			break;
		case 'h':
			usage(prg);
			return EXIT_SUCCESS;
		case 'V':
			printf("%s version %s\n", prg, PLAIN_VERSION);
			return EXIT_SUCCESS;
		case ':':
			fprintf(stderr, "Missing argument for option -%c\n",
				optopt);
			usage(prg);
			return EXIT_FAILURE;
		default:
			fprintf(stderr, "Wrong option -%c\n", optopt);
			usage(prg);
			return EXIT_FAILURE;
		}
	}

	/* Check datasize and allocate the data */
	if (datasize == 0) {
		fprintf(stderr,
			"Please specify the size of the environment "
			"partition.\n");
		usage(prg);
		return EXIT_FAILURE;
	}

	dataptr = malloc(datasize * sizeof(*dataptr));
	if (!dataptr) {
		fprintf(stderr, "Can't alloc dataptr.\n");
		return EXIT_FAILURE;
	}

	/*
	 * envptr points to the beginning of the actual environment (after the
	 * crc and possible `redundant' bit
	 */
	envsize = datasize - (CRC_SIZE + redundant);
	envptr = dataptr + CRC_SIZE + redundant;

	/* Pad the environment with the padding byte */
	memset(envptr, padbyte, envsize);

	/* Open the input file ... */
	if (optind >= argc) {
		fprintf(stderr, "Please specify an input filename\n");
		return EXIT_FAILURE;
	}

	txt_filename = argv[optind];
	if (strcmp(txt_filename, "-") == 0) {
		int readbytes = 0;
		int readlen = sizeof(*envptr) * 2048;
		txt_fd = STDIN_FILENO;

		do {
			filebuf = realloc(filebuf, readlen);
			readbytes = read(txt_fd, filebuf + filesize, readlen);
			filesize += readbytes;
		} while (readbytes == readlen);

	} else {
		txt_fd = open(txt_filename, O_RDONLY);
		if (txt_fd == -1) {
			fprintf(stderr, "Can't open \"%s\": %s\n",
					txt_filename, strerror(errno));
			return EXIT_FAILURE;
		}
		/* ... and check it */
		ret = fstat(txt_fd, &txt_file_stat);
		if (ret == -1) {
			fprintf(stderr, "Can't stat() on \"%s\": "
					"%s\n", txt_filename, strerror(errno));
			return EXIT_FAILURE;
		}

		filesize = txt_file_stat.st_size;
		/* Read the raw input file and transform it */
		filebuf = malloc(sizeof(*envptr) * filesize);
		ret = read(txt_fd, filebuf, sizeof(*envptr) * filesize);
		if (ret != sizeof(*envptr) * filesize) {
			fprintf(stderr, "Can't read the whole input file\n");
			return EXIT_FAILURE;
		}
		ret = close(txt_fd);
	}
	/*
	 * The right test to do is "=>" (not ">") because of the additional
	 * ending \0. See below.
	 */
	if (filesize >= envsize) {
		fprintf(stderr, "The input file is larger than the "
				"environment partition size\n");
		return EXIT_FAILURE;
	}

	/* Replace newlines separating variables with \0 */
	for (fp = 0, ep = 0 ; fp < filesize ; fp++) {
		if (filebuf[fp] == '\n') {
			if (ep == 0) {
				/*
				 * Newlines at the beginning of the file ?
				 * Ignore them.
				 */
				continue;
			} else if (filebuf[fp-1] == '\\') {
				/*
				 * Embedded newline in a variable.
				 *
				 * The backslash was added to the envptr; rewind
				 * and replace it with a newline
				 */
				ep--;
				envptr[ep++] = '\n';
			} else {
				/* End of a variable */
				envptr[ep++] = '\0';
			}
		} else if (filebuf[fp] == '#') {
			if (fp != 0 && filebuf[fp-1] == '\n') {
				/* This line is a comment, let's skip it */
				while (fp < txt_file_stat.st_size && fp++ &&
				       filebuf[fp] != '\n');
			} else {
				envptr[ep++] = filebuf[fp];
			}
		} else {
			envptr[ep++] = filebuf[fp];
		}
	}
	/*
	 * Make sure there is a final '\0'
	 * And do it again on the next byte to mark the end of the environment.
	 */
	if (envptr[ep-1] != '\0') {
		envptr[ep++] = '\0';
		/*
		 * The text file doesn't have an ending newline.  We need to
		 * check the env size again to make sure we have room for two \0
		 */
		if (ep >= envsize) {
			fprintf(stderr, "The environment file is too large for "
					"the target environment storage\n");
			return EXIT_FAILURE;
		}
		envptr[ep] = '\0';
	} else {
		envptr[ep] = '\0';
	}

	/* Computes the CRC and put it at the beginning of the data */
	crc = crc32(0, envptr, envsize);
	targetendian_crc = bigendian ? cpu_to_be32(crc) : cpu_to_le32(crc);

	memcpy(dataptr, &targetendian_crc, sizeof(uint32_t));

	bin_fd = creat(bin_filename, S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP);
	if (bin_fd == -1) {
		fprintf(stderr, "Can't open output file \"%s\": %s\n",
				bin_filename, strerror(errno));
		return EXIT_FAILURE;
	}

	if (write(bin_fd, dataptr, sizeof(*dataptr) * datasize) !=
			sizeof(*dataptr) * datasize) {
		fprintf(stderr, "write() failed: %s\n", strerror(errno));
		return EXIT_FAILURE;
	}

	ret = close(bin_fd);

	return ret;
}
