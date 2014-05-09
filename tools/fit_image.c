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
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include "imagetool.h"
#include "fit_common.h"
#include "mkimage.h"
#include <image.h>
#include <u-boot/crc.h>

static image_header_t header;

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
static int fit_handle_file(struct image_tool_params *params)
{
	char tmpfile[MKIMAGE_MAX_TMPFILE_LEN];
	char cmd[MKIMAGE_MAX_DTC_CMDLINE_LEN];
	int tfd, destfd = 0;
	void *dest_blob = NULL;
	struct stat sbuf;
	void *ptr;
	off_t destfd_size = 0;

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

	/* We either compile the source file, or use the existing FIT image */
	if (params->datafile) {
		/* dtc -I dts -O dtb -p 500 datafile > tmpfile */
		snprintf(cmd, sizeof(cmd), "%s %s %s > %s",
			 MKIMAGE_DTC, params->dtc, params->datafile, tmpfile);
		debug("Trying to execute \"%s\"\n", cmd);
	} else {
		snprintf(cmd, sizeof(cmd), "cp %s %s",
			 params->imagefile, tmpfile);
	}
	if (system (cmd) == -1) {
		fprintf (stderr, "%s: system(%s) failed: %s\n",
				params->cmdname, cmd, strerror(errno));
		goto err_system;
	}

	if (params->keydest) {
		destfd = mmap_fdt(params->cmdname, params->keydest,
				  &dest_blob, &sbuf, 1);
		if (destfd < 0)
			goto err_keydest;
		destfd_size = sbuf.st_size;
	}

	tfd = mmap_fdt(params->cmdname, tmpfile, &ptr, &sbuf, 1);
	if (tfd < 0)
		goto err_mmap;

	/* set hashes for images in the blob */
	if (fit_add_verification_data(params->keydir,
				      dest_blob, ptr, params->comment,
				      params->require_keys)) {
		fprintf(stderr, "%s Can't add hashes to FIT blob\n",
			params->cmdname);
		goto err_add_hashes;
	}

	/* for first image creation, add a timestamp at offset 0 i.e., root  */
	if (params->datafile && fit_set_timestamp(ptr, 0, sbuf.st_mtime)) {
		fprintf (stderr, "%s: Can't add image timestamp\n",
				params->cmdname);
		goto err_add_timestamp;
	}
	debug ("Added timestamp successfully\n");

	munmap ((void *)ptr, sbuf.st_size);
	close (tfd);
	if (dest_blob) {
		munmap(dest_blob, destfd_size);
		close(destfd);
	}

	if (rename (tmpfile, params->imagefile) == -1) {
		fprintf (stderr, "%s: Can't rename %s to %s: %s\n",
				params->cmdname, tmpfile, params->imagefile,
				strerror (errno));
		unlink (tmpfile);
		unlink (params->imagefile);
		return (EXIT_FAILURE);
	}
	return (EXIT_SUCCESS);

err_add_timestamp:
err_add_hashes:
	munmap(ptr, sbuf.st_size);
err_mmap:
	if (dest_blob)
		munmap(dest_blob, destfd_size);
err_keydest:
err_system:
	unlink(tmpfile);
	return -1;
}

static int fit_check_params(struct image_tool_params *params)
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
	register_image_type(&fitimage_params);
}
