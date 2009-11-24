/*
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

#include "mkimage.h"
#include <image.h>
#include <u-boot/crc.h>

static image_header_t header;

static int fit_verify_header (unsigned char *ptr, int image_size,
			struct mkimage_params *params)
{
	return fdt_check_header ((void *)ptr);
}

static int fit_check_image_types (uint8_t type)
{
	if (type == IH_TYPE_FLATDT)
		return EXIT_SUCCESS;
	else
		return EXIT_FAILURE;
}

/**
 * fit_handle_file - main FIT file processing function
 *
 * fit_handle_file() runs dtc to convert .its to .itb, includes
 * binary data, updates timestamp property and calculates hashes.
 *
 * datafile  - .its file
 * imagefile - .itb file
 *
 * returns:
 *     only on success, otherwise calls exit (EXIT_FAILURE);
 */
static int fit_handle_file (struct mkimage_params *params)
{
	char tmpfile[MKIMAGE_MAX_TMPFILE_LEN];
	char cmd[MKIMAGE_MAX_DTC_CMDLINE_LEN];
	int tfd;
	struct stat sbuf;
	unsigned char *ptr;

	/* Flattened Image Tree (FIT) format  handling */
	debug ("FIT format handling\n");

	/* call dtc to include binary properties into the tmp file */
	if (strlen (params->imagefile) +
		strlen (MKIMAGE_TMPFILE_SUFFIX) + 1 > sizeof (tmpfile)) {
		fprintf (stderr, "%s: Image file name (%s) too long, "
				"can't create tmpfile",
				params->imagefile, params->cmdname);
		return (EXIT_FAILURE);
	}
	sprintf (tmpfile, "%s%s", params->imagefile, MKIMAGE_TMPFILE_SUFFIX);

	/* dtc -I dts -O -p 200 datafile > tmpfile */
	sprintf (cmd, "%s %s %s > %s",
		MKIMAGE_DTC, params->dtc, params->datafile, tmpfile);
	debug ("Trying to execute \"%s\"\n", cmd);
	if (system (cmd) == -1) {
		fprintf (stderr, "%s: system(%s) failed: %s\n",
				params->cmdname, cmd, strerror(errno));
		unlink (tmpfile);
		return (EXIT_FAILURE);
	}

	/* load FIT blob into memory */
	tfd = open (tmpfile, O_RDWR|O_BINARY);

	if (tfd < 0) {
		fprintf (stderr, "%s: Can't open %s: %s\n",
				params->cmdname, tmpfile, strerror(errno));
		unlink (tmpfile);
		return (EXIT_FAILURE);
	}

	if (fstat (tfd, &sbuf) < 0) {
		fprintf (stderr, "%s: Can't stat %s: %s\n",
				params->cmdname, tmpfile, strerror(errno));
		unlink (tmpfile);
		return (EXIT_FAILURE);
	}

	ptr = mmap (0, sbuf.st_size, PROT_READ|PROT_WRITE, MAP_SHARED,
				tfd, 0);
	if (ptr == MAP_FAILED) {
		fprintf (stderr, "%s: Can't read %s: %s\n",
				params->cmdname, tmpfile, strerror(errno));
		unlink (tmpfile);
		return (EXIT_FAILURE);
	}

	/* check if ptr has a valid blob */
	if (fdt_check_header (ptr)) {
		fprintf (stderr, "%s: Invalid FIT blob\n", params->cmdname);
		unlink (tmpfile);
		return (EXIT_FAILURE);
	}

	/* set hashes for images in the blob */
	if (fit_set_hashes (ptr)) {
		fprintf (stderr, "%s Can't add hashes to FIT blob",
				params->cmdname);
		unlink (tmpfile);
		return (EXIT_FAILURE);
	}

	/* add a timestamp at offset 0 i.e., root  */
	if (fit_set_timestamp (ptr, 0, sbuf.st_mtime)) {
		fprintf (stderr, "%s: Can't add image timestamp\n",
				params->cmdname);
		unlink (tmpfile);
		return (EXIT_FAILURE);
	}
	debug ("Added timestamp successfully\n");

	munmap ((void *)ptr, sbuf.st_size);
	close (tfd);

	if (rename (tmpfile, params->imagefile) == -1) {
		fprintf (stderr, "%s: Can't rename %s to %s: %s\n",
				params->cmdname, tmpfile, params->imagefile,
				strerror (errno));
		unlink (tmpfile);
		unlink (params->imagefile);
		return (EXIT_FAILURE);
	}
	return (EXIT_SUCCESS);
}

static int fit_check_params (struct mkimage_params *params)
{
	return	((params->dflag && (params->fflag || params->lflag)) ||
		(params->fflag && (params->dflag || params->lflag)) ||
		(params->lflag && (params->dflag || params->fflag)));
}

static struct image_type_params fitimage_params = {
	.name = "FIT Image support",
	.header_size = sizeof(image_header_t),
	.hdr = (void*)&header,
	.verify_header = fit_verify_header,
	.print_header = fit_print_contents,
	.check_image_type = fit_check_image_types,
	.fflag_handle = fit_handle_file,
	.set_header = NULL,	/* FIT images use DTB header */
	.check_params = fit_check_params,
};

void init_fit_image_type (void)
{
	mkimage_register (&fitimage_params);
}
