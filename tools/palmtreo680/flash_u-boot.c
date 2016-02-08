/*
 * Copyright (C) 2013 Mike Dunn <mikedunn@newsguy.com>
 *
 * This file is released under the terms of GPL v2 and any later version.
 * See the file COPYING in the root directory of the source tree for details.
 *
 *
 * This is a userspace Linux utility that, when run on the Treo 680, will
 * program u-boot to flash.  The docg4 driver *must* be loaded with the
 * reliable_mode and ignore_badblocks parameters enabled:
 *
 *        modprobe docg4 ignore_badblocks=1 reliable_mode=1
 *
 * This utility writes the concatenated spl + u-boot image to the start of the
 * mtd device in the format expected by the IPL/SPL.  The image file and mtd
 * device node are passed to the utility as arguments.  The blocks must have
 * been erased beforehand.
 *
 * When you compile this, note that it links to libmtd from mtd-utils, so ensure
 * that your include and lib paths include this.
 */

#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>
#include <errno.h>
#include <mtd/mtd-user.h>
#include "libmtd.h"

#define RELIABLE_BLOCKSIZE  0x10000 /* block capacity in reliable mode */
#define STANDARD_BLOCKSIZE  0x40000 /* block capacity in normal mode */
#define PAGESIZE 512
#define PAGES_PER_BLOCK 512
#define OOBSIZE 7		/* available to user (16 total) */

uint8_t ff_oob[OOBSIZE] = {0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff};

/* this is the magic number the IPL looks for (ASCII "BIPO") */
uint8_t page0_oob[OOBSIZE] = {'B', 'I', 'P', 'O', 0xff, 0xff, 0xff};

int main(int argc, char * const argv[])
{
	int devfd, datafd, num_blocks, block;
	off_t file_size;
	libmtd_t mtd_desc;
	struct mtd_dev_info devinfo;
	uint8_t *blockbuf;
	char response[8];

	if (argc != 3) {
		printf("usage: %s <image file> <mtd dev node>\n", argv[0]);
		return -EINVAL;
	}

	mtd_desc = libmtd_open();
	if (mtd_desc == NULL) {
		int errsv = errno;
		fprintf(stderr, "can't initialize libmtd\n");
		return -errsv;
	}

	/* open the spl image file and mtd device */
	datafd = open(argv[1], O_RDONLY);
	if (datafd == -1) {
		int errsv = errno;
		perror(argv[1]);
		return -errsv;
	}
	devfd = open(argv[2], O_WRONLY);
	if (devfd == -1) {
		int errsv = errno;
		perror(argv[2]);
		return -errsv;
	}
	if (mtd_get_dev_info(mtd_desc, argv[2], &devinfo) < 0) {
		int errsv = errno;
		perror(argv[2]);
		return -errsv;
	}

	/* determine the number of blocks needed by the image */
	file_size = lseek(datafd, 0, SEEK_END);
	if (file_size == (off_t)-1) {
		int errsv = errno;
		perror("lseek");
		return -errsv;
	}
	num_blocks = (file_size + RELIABLE_BLOCKSIZE - 1) / RELIABLE_BLOCKSIZE;
	file_size = lseek(datafd, 0, SEEK_SET);
	if (file_size == (off_t)-1) {
		int errsv = errno;
		perror("lseek");
		return -errsv;
	}
	printf("The mtd partition contains %d blocks\n", devinfo.eb_cnt);
	printf("U-Boot will occupy %d blocks\n", num_blocks);
	if (num_blocks > devinfo.eb_cnt) {
		fprintf(stderr, "Insufficient blocks on partition\n");
		return -EINVAL;
	}

	printf("IMPORTANT: These blocks must be in an erased state!\n");
	printf("Do you want to proceed?\n");
	scanf("%s", response);
	if ((response[0] != 'y') && (response[0] != 'Y')) {
		printf("Exiting\n");
		close(devfd);
		close(datafd);
		return 0;
	}

	blockbuf = calloc(RELIABLE_BLOCKSIZE, 1);
	if (blockbuf == NULL) {
		int errsv = errno;
		perror("calloc");
		return -errsv;
	}

	for (block = 0; block < num_blocks; block++) {
		int ofs, page;
		uint8_t *pagebuf = blockbuf, *buf = blockbuf;
		uint8_t *oobbuf = page0_oob; /* magic num in oob of 1st page */
		size_t len = RELIABLE_BLOCKSIZE;
		int ret;

		/* read data for one block from file */
		while (len) {
			ssize_t read_ret = read(datafd, buf, len);
			if (read_ret == -1) {
				int errsv = errno;
				if (errno == EINTR)
					continue;
				perror("read");
				return -errsv;
			} else if (read_ret == 0) {
				break; /* EOF */
			}
			len -= read_ret;
			buf += read_ret;
		}

		printf("Block %d: writing\r", block + 1);
		fflush(stdout);

		for (page = 0, ofs = 0;
		     page < PAGES_PER_BLOCK;
		     page++, ofs += PAGESIZE) {
			if (page & 0x04)  /* Odd-numbered 2k page */
				continue; /* skipped in reliable mode */

			ret = mtd_write(mtd_desc, &devinfo, devfd, block, ofs,
					pagebuf, PAGESIZE, oobbuf, OOBSIZE,
					MTD_OPS_PLACE_OOB);
			if (ret) {
				fprintf(stderr,
					"\nmtd_write returned %d on block %d, ofs %x\n",
					ret, block + 1, ofs);
				return -EIO;
			}
			oobbuf = ff_oob;  /* oob for subsequent pages */

			if (page & 0x01)  /* odd-numbered subpage */
				pagebuf += PAGESIZE;
		}
	}

	printf("\nDone\n");

	close(devfd);
	close(datafd);
	free(blockbuf);
	return 0;
}
