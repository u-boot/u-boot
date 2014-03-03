/*
 * (C) Copyright 2014
 * DENX Software Engineering
 * Heiko Schocher <hs@denx.de>
 *
 * (C) Copyright 2008 Semihalf
 *
 * (C) Copyright 2000-2004
 * DENX Software Engineering
 * Wolfgang Denk, wd@denx.de
 *
 * Updated-by: Prafulla Wadaskar <prafulla@marvell.com>
 *		FIT image specific code abstracted from mkimage.c
 *		some functions added to address abstraction
 *
 * All rights reserved.
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include "imagetool.h"
#include "mkimage.h"
#include "fit_common.h"
#include <image.h>
#include <u-boot/crc.h>

int fit_verify_header(unsigned char *ptr, int image_size,
			struct image_tool_params *params)
{
	return fdt_check_header(ptr);
}

int fit_check_image_types(uint8_t type)
{
	if (type == IH_TYPE_FLATDT)
		return EXIT_SUCCESS;
	else
		return EXIT_FAILURE;
}

int mmap_fdt(char *cmdname, const char *fname, void **blobp,
		struct stat *sbuf, int useunlink)
{
	void *ptr;
	int fd;

	/* Load FIT blob into memory (we need to write hashes/signatures) */
	fd = open(fname, O_RDWR | O_BINARY);

	if (fd < 0) {
		fprintf(stderr, "%s: Can't open %s: %s\n",
			cmdname, fname, strerror(errno));
		if (useunlink)
			unlink(fname);
		return -1;
	}

	if (fstat(fd, sbuf) < 0) {
		fprintf(stderr, "%s: Can't stat %s: %s\n",
			cmdname, fname, strerror(errno));
		if (useunlink)
			unlink(fname);
		return -1;
	}

	errno = 0;
	ptr = mmap(0, sbuf->st_size, PROT_READ|PROT_WRITE, MAP_SHARED, fd, 0);
	if ((ptr == MAP_FAILED) || (errno != 0)) {
		fprintf(stderr, "%s: Can't read %s: %s\n",
			cmdname, fname, strerror(errno));
		if (useunlink)
			unlink(fname);
		return -1;
	}

	/* check if ptr has a valid blob */
	if (fdt_check_header(ptr)) {
		fprintf(stderr, "%s: Invalid FIT blob\n", cmdname);
		if (useunlink)
			unlink(fname);
		return -1;
	}

	*blobp = ptr;
	return fd;
}
