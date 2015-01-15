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

static int fit_add_file_data(struct image_tool_params *params, size_t size_inc,
			     const char *tmpfile)
{
	int tfd, destfd = 0;
	void *dest_blob = NULL;
	off_t destfd_size = 0;
	struct stat sbuf;
	void *ptr;
	int ret = 0;

	tfd = mmap_fdt(params->cmdname, tmpfile, size_inc, &ptr, &sbuf, true);
	if (tfd < 0)
		return -EIO;

	if (params->keydest) {
		struct stat dest_sbuf;

		destfd = mmap_fdt(params->cmdname, params->keydest, size_inc,
				  &dest_blob, &dest_sbuf, false);
		if (destfd < 0) {
			ret = -EIO;
			goto err_keydest;
		}
		destfd_size = dest_sbuf.st_size;
	}

	/* for first image creation, add a timestamp at offset 0 i.e., root  */
	if (params->datafile)
		ret = fit_set_timestamp(ptr, 0, sbuf.st_mtime);

	if (!ret) {
		ret = fit_add_verification_data(params->keydir, dest_blob, ptr,
						params->comment,
						params->require_keys);
	}

	if (dest_blob) {
		munmap(dest_blob, destfd_size);
		close(destfd);
	}

err_keydest:
	munmap(ptr, sbuf.st_size);
	close(tfd);

	return ret;
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
static int fit_handle_file(struct image_tool_params *params)
{
	char tmpfile[MKIMAGE_MAX_TMPFILE_LEN];
	char cmd[MKIMAGE_MAX_DTC_CMDLINE_LEN];
	size_t size_inc;
	int ret;

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

	/*
	 * Set hashes for images in the blob. Unfortunately we may need more
	 * space in either FDT, so keep trying until we succeed.
	 *
	 * Note: this is pretty inefficient for signing, since we must
	 * calculate the signature every time. It would be better to calculate
	 * all the data and then store it in a separate step. However, this
	 * would be considerably more complex to implement. Generally a few
	 * steps of this loop is enough to sign with several keys.
	 */
	for (size_inc = 0; size_inc < 64 * 1024; size_inc += 1024) {
		ret = fit_add_file_data(params, size_inc, tmpfile);
		if (!ret || ret != -ENOSPC)
			break;
	}

	if (ret) {
		fprintf(stderr, "%s Can't add hashes to FIT blob\n",
			params->cmdname);
		goto err_system;
	}

	if (rename (tmpfile, params->imagefile) == -1) {
		fprintf (stderr, "%s: Can't rename %s to %s: %s\n",
				params->cmdname, tmpfile, params->imagefile,
				strerror (errno));
		unlink (tmpfile);
		unlink (params->imagefile);
		return EXIT_FAILURE;
	}
	return EXIT_SUCCESS;

err_system:
	unlink(tmpfile);
	return -1;
}

/**
 * fit_image_extract - extract a FIT component image
 * @fit: pointer to the FIT format image header
 * @image_noffset: offset of the component image node
 * @file_name: name of the file to store the FIT sub-image
 *
 * returns:
 *     zero in case of success or a negative value if fail.
 */
static int fit_image_extract(
	const void *fit,
	int image_noffset,
	const char *file_name)
{
	const void *file_data;
	size_t file_size = 0;

	/* get the "data" property of component at offset "image_noffset" */
	fit_image_get_data(fit, image_noffset, &file_data, &file_size);

	/* save the "file_data" into the file specified by "file_name" */
	return imagetool_save_subimage(file_name, (ulong) file_data, file_size);
}

/**
 * fit_extract_contents - retrieve a sub-image component from the FIT image
 * @ptr: pointer to the FIT format image header
 * @params: command line parameters
 *
 * returns:
 *     zero in case of success or a negative value if fail.
 */
static int fit_extract_contents(void *ptr, struct image_tool_params *params)
{
	int images_noffset;
	int noffset;
	int ndepth;
	const void *fit = ptr;
	int count = 0;
	const char *p;

	/* Indent string is defined in header image.h */
	p = IMAGE_INDENT_STRING;

	if (!fit_check_format(fit)) {
		printf("Bad FIT image format\n");
		return -1;
	}

	/* Find images parent node offset */
	images_noffset = fdt_path_offset(fit, FIT_IMAGES_PATH);
	if (images_noffset < 0) {
		printf("Can't find images parent node '%s' (%s)\n",
		       FIT_IMAGES_PATH, fdt_strerror(images_noffset));
		return -1;
	}

	/* Avoid any overrun */
	count = fit_get_subimage_count(fit, images_noffset);
	if ((params->pflag < 0) || (count <= params->pflag)) {
		printf("No such component at '%d'\n", params->pflag);
		return -1;
	}

	/* Process its subnodes, extract the desired component from image */
	for (ndepth = 0, count = 0,
		noffset = fdt_next_node(fit, images_noffset, &ndepth);
		(noffset >= 0) && (ndepth > 0);
		noffset = fdt_next_node(fit, noffset, &ndepth)) {
		if (ndepth == 1) {
			/*
			 * Direct child node of the images parent node,
			 * i.e. component image node.
			 */
			if (params->pflag == count) {
				printf("Extracted:\n%s Image %u (%s)\n", p,
				       count, fit_get_name(fit, noffset, NULL));

				fit_image_print(fit, noffset, p);

				return fit_image_extract(fit, noffset,
						params->outfile);
			}

			count++;
		}
	}

	return 0;
}

static int fit_check_params(struct image_tool_params *params)
{
	return	((params->dflag && (params->fflag || params->lflag)) ||
		(params->fflag && (params->dflag || params->lflag)) ||
		(params->lflag && (params->dflag || params->fflag)));
}

U_BOOT_IMAGE_TYPE(
	fitimage,
	"FIT Image support",
	sizeof(image_header_t),
	(void *)&header,
	fit_check_params,
	fit_verify_header,
	fit_print_contents,
	NULL,
	fit_extract_contents,
	fit_check_image_types,
	fit_handle_file,
	NULL /* FIT images use DTB header */
);
