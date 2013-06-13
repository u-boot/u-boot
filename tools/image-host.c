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
#include <image.h>
#include <version.h>

/**
 * fit_set_hash_value - set hash value in requested has node
 * @fit: pointer to the FIT format image header
 * @noffset: hash node offset
 * @value: hash value to be set
 * @value_len: hash value length
 *
 * fit_set_hash_value() attempts to set hash value in a node at offset
 * given and returns operation status to the caller.
 *
 * returns
 *     0, on success
 *     -1, on failure
 */
static int fit_set_hash_value(void *fit, int noffset, uint8_t *value,
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
	const char *node_name;
	int value_len;
	char *algo;

	node_name = fit_get_name(fit, noffset, NULL);

	if (fit_image_hash_get_algo(fit, noffset, &algo)) {
		printf("Can't get hash algo property for '%s' hash node in '%s' image node\n",
		       node_name, image_name);
		return -1;
	}

	if (calculate_hash(data, size, algo, value, &value_len)) {
		printf("Unsupported hash algorithm (%s) for '%s' hash node in '%s' image node\n",
		       algo, node_name, image_name);
		return -1;
	}

	if (fit_set_hash_value(fit, noffset, value, value_len)) {
		printf("Can't set hash value for '%s' hash node in '%s' image node\n",
		       node_name, image_name);
		return -1;
	}

	return 0;
}

/**
 * fit_image_write_sig() - write the signature to a FIT
 *
 * This writes the signature and signer data to the FIT.
 *
 * @fit: pointer to the FIT format image header
 * @noffset: hash node offset
 * @value: signature value to be set
 * @value_len: signature value length
 * @comment: Text comment to write (NULL for none)
 *
 * returns
 *     0, on success
 *     -FDT_ERR_..., on failure
 */
static int fit_image_write_sig(void *fit, int noffset, uint8_t *value,
		int value_len, const char *comment, const char *region_prop,
		int region_proplen)
{
	int string_size;
	int ret;

	/*
	 * Get the current string size, before we update the FIT and add
	 * more
	 */
	string_size = fdt_size_dt_strings(fit);

	ret = fdt_setprop(fit, noffset, FIT_VALUE_PROP, value, value_len);
	if (!ret) {
		ret = fdt_setprop_string(fit, noffset, "signer-name",
					 "mkimage");
	}
	if (!ret) {
		ret = fdt_setprop_string(fit, noffset, "signer-version",
				  PLAIN_VERSION);
	}
	if (comment && !ret)
		ret = fdt_setprop_string(fit, noffset, "comment", comment);
	if (!ret)
		ret = fit_set_timestamp(fit, noffset, time(NULL));
	if (region_prop && !ret) {
		uint32_t strdata[2];

		ret = fdt_setprop(fit, noffset, "hashed-nodes",
				   region_prop, region_proplen);
		strdata[0] = 0;
		strdata[1] = cpu_to_fdt32(string_size);
		if (!ret) {
			ret = fdt_setprop(fit, noffset, "hashed-strings",
					  strdata, sizeof(strdata));
		}
	}

	return ret;
}

static int fit_image_setup_sig(struct image_sign_info *info,
		const char *keydir, void *fit, const char *image_name,
		int noffset, const char *require_keys)
{
	const char *node_name;
	char *algo_name;

	node_name = fit_get_name(fit, noffset, NULL);
	if (fit_image_hash_get_algo(fit, noffset, &algo_name)) {
		printf("Can't get algo property for '%s' signature node in '%s' image node\n",
		       node_name, image_name);
		return -1;
	}

	memset(info, '\0', sizeof(*info));
	info->keydir = keydir;
	info->keyname = fdt_getprop(fit, noffset, "key-name-hint", NULL);
	info->fit = fit;
	info->node_offset = noffset;
	info->algo = image_get_sig_algo(algo_name);
	info->require_keys = require_keys;
	if (!info->algo) {
		printf("Unsupported signature algorithm (%s) for '%s' signature node in '%s' image node\n",
		       algo_name, node_name, image_name);
		return -1;
	}

	return 0;
}

/**
 * fit_image_process_sig- Process a single subnode of the images/ node
 *
 * Check each subnode and process accordingly. For signature nodes we
 * generate a signed hash of the supplised data and store it in the node.
 *
 * @keydir:	Directory containing keys to use for signing
 * @keydest:	Destination FDT blob to write public keys into
 * @fit:	pointer to the FIT format image header
 * @image_name:	name of image being processes (used to display errors)
 * @noffset:	subnode offset
 * @data:	data to process
 * @size:	size of data in bytes
 * @comment:	Comment to add to signature nodes
 * @require_keys: Mark all keys as 'required'
 * @return 0 if ok, -1 on error
 */
static int fit_image_process_sig(const char *keydir, void *keydest,
		void *fit, const char *image_name,
		int noffset, const void *data, size_t size,
		const char *comment, int require_keys)
{
	struct image_sign_info info;
	struct image_region region;
	const char *node_name;
	uint8_t *value;
	uint value_len;
	int ret;

	if (fit_image_setup_sig(&info, keydir, fit, image_name, noffset,
				require_keys ? "image" : NULL))
		return -1;

	node_name = fit_get_name(fit, noffset, NULL);
	region.data = data;
	region.size = size;
	ret = info.algo->sign(&info, &region, 1, &value, &value_len);
	if (ret) {
		printf("Failed to sign '%s' signature node in '%s' image node: %d\n",
		       node_name, image_name, ret);

		/* We allow keys to be missing */
		if (ret == -ENOENT)
			return 0;
		return -1;
	}

	ret = fit_image_write_sig(fit, noffset, value, value_len, comment,
			NULL, 0);
	if (ret) {
		printf("Can't write signature for '%s' signature node in '%s' image node: %s\n",
		       node_name, image_name, fdt_strerror(ret));
		return -1;
	}
	free(value);

	/* Get keyname again, as FDT has changed and invalidated our pointer */
	info.keyname = fdt_getprop(fit, noffset, "key-name-hint", NULL);

	/* Write the public key into the supplied FDT file */
	if (keydest && info.algo->add_verify_data(&info, keydest)) {
		printf("Failed to add verification data for '%s' signature node in '%s' image node\n",
		       node_name, image_name);
		return -1;
	}

	return 0;
}

/**
 * fit_image_add_verification_data() - calculate/set verig. data for image node
 *
 * This adds hash and signature values for an component image node.
 *
 * All existing hash subnodes are checked, if algorithm property is set to
 * one of the supported hash algorithms, hash value is computed and
 * corresponding hash node property is set, for example:
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
 * For signature details, please see doc/uImage.FIT/signature.txt
 *
 * @keydir	Directory containing *.key and *.crt files (or NULL)
 * @keydest	FDT Blob to write public keys into (NULL if none)
 * @fit:	Pointer to the FIT format image header
 * @image_noffset: Requested component image node
 * @comment:	Comment to add to signature nodes
 * @require_keys: Mark all keys as 'required'
 * @return: 0 on success, <0 on failure
 */
int fit_image_add_verification_data(const char *keydir, void *keydest,
		void *fit, int image_noffset, const char *comment,
		int require_keys)
{
	const char *image_name;
	const void *data;
	size_t size;
	int noffset;

	/* Get image data and data length */
	if (fit_image_get_data(fit, image_noffset, &data, &size)) {
		printf("Can't get image data/size\n");
		return -1;
	}

	image_name = fit_get_name(fit, image_noffset, NULL);

	/* Process all hash subnodes of the component image node */
	for (noffset = fdt_first_subnode(fit, image_noffset);
	     noffset >= 0;
	     noffset = fdt_next_subnode(fit, noffset)) {
		const char *node_name;
		int ret = 0;

		/*
		 * Check subnode name, must be equal to "hash" or "signature".
		 * Multiple hash nodes require unique unit node
		 * names, e.g. hash@1, hash@2, signature@1, etc.
		 */
		node_name = fit_get_name(fit, noffset, NULL);
		if (!strncmp(node_name, FIT_HASH_NODENAME,
			     strlen(FIT_HASH_NODENAME))) {
			ret = fit_image_process_hash(fit, image_name, noffset,
						data, size);
		} else if (IMAGE_ENABLE_SIGN && keydir &&
			   !strncmp(node_name, FIT_SIG_NODENAME,
				strlen(FIT_SIG_NODENAME))) {
			ret = fit_image_process_sig(keydir, keydest,
				fit, image_name, noffset, data, size,
				comment, require_keys);
		}
		if (ret)
			return -1;
	}

	return 0;
}

int fit_add_verification_data(const char *keydir, void *keydest, void *fit,
			      const char *comment, int require_keys)
{
	int images_noffset;
	int noffset;
	int ret;

	/* Find images parent node offset */
	images_noffset = fdt_path_offset(fit, FIT_IMAGES_PATH);
	if (images_noffset < 0) {
		printf("Can't find images parent node '%s' (%s)\n",
		       FIT_IMAGES_PATH, fdt_strerror(images_noffset));
		return images_noffset;
	}

	/* Process its subnodes, print out component images details */
	for (noffset = fdt_first_subnode(fit, images_noffset);
	     noffset >= 0;
	     noffset = fdt_next_subnode(fit, noffset)) {
		/*
		 * Direct child node of the images parent node,
		 * i.e. component image node.
		 */
		ret = fit_image_add_verification_data(keydir, keydest,
				fit, noffset, comment, require_keys);
		if (ret)
			return ret;
	}

	return 0;
}
