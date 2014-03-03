/*
 * (C) Copyright 2014
 * DENX Software Engineering
 * Heiko Schocher <hs@denx.de>
 *
 * Based on:
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

#include "mkimage.h"
#include "fit_common.h"
#include <image.h>
#include <u-boot/crc.h>

void usage(char *cmdname)
{
	fprintf(stderr, "Usage: %s -f fit file -k key file\n"
			 "          -f ==> set fit file which should be checked'\n"
			 "          -k ==> set key file which contains the key'\n",
		cmdname);
	exit(EXIT_FAILURE);
}

int main(int argc, char **argv)
{
	int ffd = -1;
	int kfd = -1;
	struct stat fsbuf;
	struct stat ksbuf;
	void *fit_blob;
	char *fdtfile = NULL;
	char *keyfile = NULL;
	char cmdname[50];
	int ret;
	void *key_blob;
	int c;

	strcpy(cmdname, *argv);
	while ((c = getopt(argc, argv, "f:k:")) != -1)
		switch (c) {
		case 'f':
			fdtfile = optarg;
			break;
		case 'k':
			keyfile = optarg;
			break;
		default:
			usage(cmdname);
			break;
	}

	ffd = mmap_fdt(cmdname, fdtfile, &fit_blob, &fsbuf, 0);
	if (ffd < 0)
		return EXIT_FAILURE;
	kfd = mmap_fdt(cmdname, keyfile, &key_blob, &ksbuf, 0);
	if (ffd < 0)
		return EXIT_FAILURE;

	image_set_host_blob(key_blob);
	ret = fit_check_sign(fit_blob, key_blob);

	if (ret)
		ret = EXIT_SUCCESS;
	else
		ret = EXIT_FAILURE;

	(void) munmap((void *)fit_blob, fsbuf.st_size);
	(void) munmap((void *)key_blob, ksbuf.st_size);

	close(ffd);
	close(kfd);
	exit(ret);
}
