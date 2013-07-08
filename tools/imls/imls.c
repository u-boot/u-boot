/*
 * (C) Copyright 2009 Marco Stornelli
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>
#include <string.h>
#include <sys/types.h>
#include <sys/ioctl.h>
#include <sys/stat.h>
#include <unistd.h>
#include <asm/page.h>

#ifdef MTD_OLD
#include <stdint.h>
#include <linux/mtd/mtd.h>
#else
#define  __user	/* nothing */
#include <mtd/mtd-user.h>
#endif

#include <sha1.h>
#include <libfdt.h>
#include <fdt_support.h>
#include <image.h>

#define MIN(a, b) (((a) < (b)) ? (a) : (b))

extern unsigned long crc32(unsigned long crc, const char *buf, unsigned int len);
static void usage(void);
static int image_verify_header(char *ptr, int fd);
static int flash_bad_block(int fd, uint8_t mtd_type, loff_t start);

char	*cmdname;
char	*devicefile;

unsigned int sectorcount = 0;
int sflag = 0;
unsigned int sectoroffset = 0;
unsigned int sectorsize = 0;
int cflag = 0;

int main (int argc, char **argv)
{
	int fd = -1, err = 0, readbyte = 0, j;
	struct mtd_info_user mtdinfo;
	char buf[sizeof(image_header_t)];
	int found = 0;

	cmdname = *argv;

	while (--argc > 0 && **++argv == '-') {
		while (*++*argv) {
			switch (**argv) {
			case 'c':
				if (--argc <= 0)
					usage ();
				sectorcount = (unsigned int)atoi(*++argv);
				cflag = 1;
				goto NXTARG;
			case 'o':
				if (--argc <= 0)
					usage ();
				sectoroffset = (unsigned int)atoi(*++argv);
				goto NXTARG;

			case 's':
				if (--argc <= 0)
					usage ();
				sectorsize = (unsigned int)atoi(*++argv);
				sflag = 1;
				goto NXTARG;
			default:
				usage ();
			}
		}
NXTARG:		;
	}

	if (argc != 1 || cflag == 0 || sflag == 0)
		usage();

	devicefile = *argv;

	fd = open(devicefile, O_RDONLY);
	if (fd < 0) {
		fprintf (stderr, "%s: Can't open %s: %s\n",
			 cmdname, devicefile, strerror(errno));
		exit(EXIT_FAILURE);
	}

	err = ioctl(fd, MEMGETINFO, &mtdinfo);
	if (err < 0) {
		fprintf(stderr, "%s: Cannot get MTD information: %s\n",cmdname,
			strerror(errno));
		exit(EXIT_FAILURE);
	}

	if (mtdinfo.type != MTD_NORFLASH && mtdinfo.type != MTD_NANDFLASH) {
		fprintf(stderr, "%s: Unsupported flash type %u\n",
			cmdname, mtdinfo.type);
		exit(EXIT_FAILURE);
	}

	if (sectorsize * sectorcount != mtdinfo.size) {
		fprintf(stderr, "%s: Partition size (%d) incompatible with "
			"sector size and count\n", cmdname, mtdinfo.size);
		exit(EXIT_FAILURE);
	}

	if (sectorsize * sectoroffset >= mtdinfo.size) {
		fprintf(stderr, "%s: Partition size (%d) incompatible with "
			"sector offset given\n", cmdname, mtdinfo.size);
		exit(EXIT_FAILURE);
	}

	if (sectoroffset > sectorcount - 1) {
		fprintf(stderr, "%s: Sector offset cannot be grater than "
			"sector count minus one\n", cmdname);
		exit(EXIT_FAILURE);
	}

	printf("Searching....\n");

	for (j = sectoroffset; j < sectorcount; ++j) {

		if (lseek(fd, j*sectorsize, SEEK_SET) != j*sectorsize) {
			fprintf(stderr, "%s: lseek failure: %s\n",
			cmdname, strerror(errno));
			exit(EXIT_FAILURE);
		}

		err = flash_bad_block(fd, mtdinfo.type, j*sectorsize);
		if (err < 0)
			exit(EXIT_FAILURE);
		if (err)
			continue; /* Skip and jump to next */

		readbyte = read(fd, buf, sizeof(image_header_t));
		if (readbyte != sizeof(image_header_t)) {
			fprintf(stderr, "%s: Can't read from device: %s\n",
			cmdname, strerror(errno));
			exit(EXIT_FAILURE);
		}

		if (fdt_check_header(buf)) {
			/* old-style image */
			if (image_verify_header(buf, fd)) {
				found = 1;
				image_print_contents((image_header_t *)buf);
			}
		} else {
			/* FIT image */
			fit_print_contents(buf);
		}

	}

	close(fd);

	if(!found)
		printf("No images found\n");

	exit(EXIT_SUCCESS);
}

void usage()
{
	fprintf (stderr, "Usage:\n"
			 "       %s [-o offset] -s size -c count device\n"
			 "          -o ==> number of sectors to use as offset\n"
			 "          -c ==> number of sectors\n"
			 "          -s ==> size of sectors (byte)\n",
		cmdname);

	exit(EXIT_FAILURE);
}

static int image_verify_header(char *ptr, int fd)
{
	int len, nread;
	char *data;
	uint32_t checksum;
	image_header_t *hdr = (image_header_t *)ptr;
	char buf[PAGE_SIZE];

	if (image_get_magic(hdr) != IH_MAGIC)
		return 0;

	data = (char *)hdr;
	len  = image_get_header_size();

	checksum = image_get_hcrc(hdr);
	hdr->ih_hcrc = htonl(0);	/* clear for re-calculation */

	if (crc32(0, data, len) != checksum) {
		fprintf(stderr,
		      "%s: Maybe image found but it has bad header checksum!\n",
		      cmdname);
		return 0;
	}

	len = image_get_size(hdr);
	checksum = 0;

	while (len > 0) {
		nread = read(fd, buf, MIN(len,PAGE_SIZE));
		if (nread != MIN(len,PAGE_SIZE)) {
			fprintf(stderr,
				"%s: Error while reading: %s\n",
				cmdname, strerror(errno));
			exit(EXIT_FAILURE);
		}
		checksum = crc32(checksum, buf, nread);
		len -= nread;
	}

	if (checksum != image_get_dcrc(hdr)) {
		fprintf (stderr,
			"%s: Maybe image found but it has corrupted data!\n",
			cmdname);
		return 0;
	}

	return 1;
}

/*
 * Test for bad block on NAND, just returns 0 on NOR, on NAND:
 * 0	- block is good
 * > 0	- block is bad
 * < 0	- failed to test
 */
static int flash_bad_block(int fd, uint8_t mtd_type, loff_t start)
{
	if (mtd_type == MTD_NANDFLASH) {
		int badblock = ioctl(fd, MEMGETBADBLOCK, &start);

		if (badblock < 0) {
			fprintf(stderr,"%s: Cannot read bad block mark: %s\n",
				cmdname, strerror(errno));
			return badblock;
		}

		if (badblock) {
			return badblock;
		}
	}

	return 0;
}
