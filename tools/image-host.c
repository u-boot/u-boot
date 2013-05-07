/*
 * Copyright (c) 2013, Google Inc.
 *
 * (C) Copyright 2008 Semihalf
 *
 * (C) Copyright 2000-2006
 * Wolfgang Denk, DENX Software Engineering, wd@denx.de.
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

#include "mkimage.h"
#include <bootstage.h>
#include <image.h>
#include <sha1.h>
#include <time.h>
#include <u-boot/crc.h>
#include <u-boot/md5.h>

/**
 * fit_set_hashes - process FIT component image nodes and calculate hashes
 * @fit: pointer to the FIT format image header
 *
 * fit_set_hashes() adds hash values for all component images in the FIT blob.
 * Hashes are calculated for all component images which have hash subnodes
 * with algorithm property set to one of the supported hash algorithms.
 *
 * returns
 *     0, on success
 *     libfdt error code, on failure
 */
int fit_set_hashes(void *fit)
{
	int images_noffset;
	int noffset;
	int ndepth;
	int ret;

	/* Find images parent node offset */
	images_noffset = fdt_path_offset(fit, FIT_IMAGES_PATH);
	if (images_noffset < 0) {
		printf("Can't find images parent node '%s' (%s)\n",
		       FIT_IMAGES_PATH, fdt_strerror(images_noffset));
		return images_noffset;
	}

	/* Process its subnodes, print out component images details */
	for (ndepth = 0, noffset = fdt_next_node(fit, images_noffset, &ndepth);
	     (noffset >= 0) && (ndepth > 0);
	     noffset = fdt_next_node(fit, noffset, &ndepth)) {
		if (ndepth == 1) {
			/*
			 * Direct child node of the images parent node,
			 * i.e. component image node.
			 */
			ret = fit_image_set_hashes(fit, noffset);
			if (ret)
				return ret;
		}
	}

	return 0;
}

/**
 * fit_image_process_hash - Process a single subnode of the images/ node
 *
 * Check each subnode and process accordingly. For hash nodes we generate
 * a hash of the supplised data and store it in the node.
 *
 * @fit:	pointer to the FIT format image header
 * @image_name:	name of image being processes (used to display errors)
 * @noffset:	subnode offset
 * @data:	data to process
 * @size:	size of data in bytes
 * @return 0 if ok, -1 on error
 */
static int fit_image_process_hash(void *fit, const char *image_name,
		int noffset, const void *data, size_t size)
{
	uint8_t value[FIT_MAX_HASH_LEN];
	int value_len;
	char *algo;

	/*
	 * Check subnode name, must be equal to "hash".
	 * Multiple hash nodes require unique unit node
	 * names, e.g. hash@1, hash@2, etc.
	 */
	if (strncmp(fit_get_name(fit, noffset, NULL),
		    FIT_HASH_NODENAME, strlen(FIT_HASH_NODENAME)) != 0)
		return 0;

	if (fit_image_hash_get_algo(fit, noffset, &algo)) {
		printf("Can't get hash algo property for '%s' hash node in '%s' image node\n",
		       fit_get_name(fit, noffset, NULL), image_name);
		return -1;
	}

	if (calculate_hash(data, size, algo, value, &value_len)) {
		printf("Unsupported hash algorithm (%s) for '%s' hash node in '%s' image node\n",
		       algo, fit_get_name(fit, noffset, NULL), image_name);
		return -1;
	}

	if (fit_image_hash_set_value(fit, noffset, value, value_len)) {
		printf("Can't set hash value for '%s' hash node in '%s' image node\n",
		       fit_get_name(fit, noffset, NULL), image_name);
		return -1;
	}

	return 0;
}

/**
 * fit_image_set_hashes - calculate/set hashes for given component image node
 * @fit: pointer to the FIT format image header
 * @image_noffset: requested component image node
 *
 * fit_image_set_hashes() adds hash values for an component image node. All
 * existing hash subnodes are checked, if algorithm property is set to one of
 * the supported hash algorithms, hash value is computed and corresponding
 * hash node property is set, for example:
 *
 * Input component image node structure:
 *
 * o image@1 (at image_noffset)
 *   | - data = [binary data]
 *   o hash@1
 *     |- algo = "sha1"
 *
 * Output component image node structure:
 *
 * o image@1 (at image_noffset)
 *   | - data = [binary data]
 *   o hash@1
 *     |- algo = "sha1"
 *     |- value = sha1(data)
 *
 * returns:
 *     0 on sucess
 *    <0 on failure
 */
int fit_image_set_hashes(void *fit, int image_noffset)
{
	const void *data;
	size_t size;
	int noffset;
	int ndepth;
	const char *image_name;

	/* Get image data and data length */
	if (fit_image_get_data(fit, image_noffset, &data, &size)) {
		printf("Can't get image data/size\n");
		return -1;
	}

	image_name = fit_get_name(fit, image_noffset, NULL);

	/* Process all hash subnodes of the component image node */
	for (ndepth = 0, noffset = fdt_next_node(fit, image_noffset, &ndepth);
			(noffset >= 0) && (ndepth > 0);
			noffset = fdt_next_node(fit, noffset, &ndepth)) {
		if (ndepth == 1) {
			/* Direct child node of the component image node */
			if (fit_image_process_hash(fit, image_name, noffset,
						   data, size))
				return -1;
		}
	}

	return 0;
}

/**
 * fit_image_hash_set_value - set hash value in requested has node
 * @fit: pointer to the FIT format image header
 * @noffset: hash node offset
 * @value: hash value to be set
 * @value_len: hash value length
 *
 * fit_image_hash_set_value() attempts to set hash value in a node at offset
 * given and returns operation status to the caller.
 *
 * returns
 *     0, on success
 *     -1, on failure
 */
int fit_image_hash_set_value(void *fit, int noffset, uint8_t *value,
				int value_len)
{
	int ret;

	ret = fdt_setprop(fit, noffset, FIT_VALUE_PROP, value, value_len);
	if (ret) {
		printf("Can't set hash '%s' property for '%s' node(%s)\n",
		       FIT_VALUE_PROP, fit_get_name(fit, noffset, NULL),
		       fdt_strerror(ret));
		return -1;
	}

	return 0;
}
