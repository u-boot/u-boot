/*
 * Copyright (c) 2013, Google Inc.
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#ifdef USE_HOSTCC
#include "mkimage.h"
#include <time.h>
#else
#include <common.h>
#include <malloc.h>
DECLARE_GLOBAL_DATA_PTR;
#endif /* !USE_HOSTCC*/
#include <image.h>
#include <u-boot/rsa.h>
#include <u-boot/rsa-checksum.h>

#define IMAGE_MAX_HASHED_NODES		100

#ifdef USE_HOSTCC
void *host_blob;
void image_set_host_blob(void *blob)
{
	host_blob = blob;
}
void *image_get_host_blob(void)
{
	return host_blob;
}
#endif

struct checksum_algo checksum_algos[] = {
	{
		"sha1",
		SHA1_SUM_LEN,
		RSA2048_BYTES,
#if IMAGE_ENABLE_SIGN
		EVP_sha1,
#endif
		sha1_calculate,
		padding_sha1_rsa2048,
	},
	{
		"sha256",
		SHA256_SUM_LEN,
		RSA2048_BYTES,
#if IMAGE_ENABLE_SIGN
		EVP_sha256,
#endif
		sha256_calculate,
		padding_sha256_rsa2048,
	},
	{
		"sha256",
		SHA256_SUM_LEN,
		RSA4096_BYTES,
#if IMAGE_ENABLE_SIGN
		EVP_sha256,
#endif
		sha256_calculate,
		padding_sha256_rsa4096,
	}

};

struct image_sig_algo image_sig_algos[] = {
	{
		"sha1,rsa2048",
		rsa_sign,
		rsa_add_verify_data,
		rsa_verify,
		&checksum_algos[0],
	},
	{
		"sha256,rsa2048",
		rsa_sign,
		rsa_add_verify_data,
		rsa_verify,
		&checksum_algos[1],
	},
	{
		"sha256,rsa4096",
		rsa_sign,
		rsa_add_verify_data,
		rsa_verify,
		&checksum_algos[2],
	}

};

struct image_sig_algo *image_get_sig_algo(const char *name)
{
	int i;

	for (i = 0; i < ARRAY_SIZE(image_sig_algos); i++) {
		if (!strcmp(image_sig_algos[i].name, name))
			return &image_sig_algos[i];
	}

	return NULL;
}

/**
 * fit_region_make_list() - Make a list of image regions
 *
 * Given a list of fdt_regions, create a list of image_regions. This is a
 * simple conversion routine since the FDT and image code use different
 * structures.
 *
 * @fit: FIT image
 * @fdt_regions: Pointer to FDT regions
 * @count: Number of FDT regions
 * @region: Pointer to image regions, which must hold @count records. If
 * region is NULL, then (except for an SPL build) the array will be
 * allocated.
 * @return: Pointer to image regions
 */
struct image_region *fit_region_make_list(const void *fit,
		struct fdt_region *fdt_regions, int count,
		struct image_region *region)
{
	int i;

	debug("Hash regions:\n");
	debug("%10s %10s\n", "Offset", "Size");

	/*
	 * Use malloc() except in SPL (to save code size). In SPL the caller
	 * must allocate the array.
	 */
#ifndef CONFIG_SPL_BUILD
	if (!region)
		region = calloc(sizeof(*region), count);
#endif
	if (!region)
		return NULL;
	for (i = 0; i < count; i++) {
		debug("%10x %10x\n", fdt_regions[i].offset,
		      fdt_regions[i].size);
		region[i].data = fit + fdt_regions[i].offset;
		region[i].size = fdt_regions[i].size;
	}

	return region;
}

static int fit_image_setup_verify(struct image_sign_info *info,
		const void *fit, int noffset, int required_keynode,
		char **err_msgp)
{
	char *algo_name;

	if (fit_image_hash_get_algo(fit, noffset, &algo_name)) {
		*err_msgp = "Can't get hash algo property";
		return -1;
	}
	memset(info, '\0', sizeof(*info));
	info->keyname = fdt_getprop(fit, noffset, "key-name-hint", NULL);
	info->fit = (void *)fit;
	info->node_offset = noffset;
	info->algo = image_get_sig_algo(algo_name);
	info->fdt_blob = gd_fdt_blob();
	info->required_keynode = required_keynode;
	printf("%s:%s", algo_name, info->keyname);

	if (!info->algo) {
		*err_msgp = "Unknown signature algorithm";
		return -1;
	}

	return 0;
}

int fit_image_check_sig(const void *fit, int noffset, const void *data,
		size_t size, int required_keynode, char **err_msgp)
{
	struct image_sign_info info;
	struct image_region region;
	uint8_t *fit_value;
	int fit_value_len;

	*err_msgp = NULL;
	if (fit_image_setup_verify(&info, fit, noffset, required_keynode,
				   err_msgp))
		return -1;

	if (fit_image_hash_get_value(fit, noffset, &fit_value,
				     &fit_value_len)) {
		*err_msgp = "Can't get hash value property";
		return -1;
	}

	region.data = data;
	region.size = size;

	if (info.algo->verify(&info, &region, 1, fit_value, fit_value_len)) {
		*err_msgp = "Verification failed";
		return -1;
	}

	return 0;
}

static int fit_image_verify_sig(const void *fit, int image_noffset,
		const char *data, size_t size, const void *sig_blob,
		int sig_offset)
{
	int noffset;
	char *err_msg = "";
	int verified = 0;
	int ret;

	/* Process all hash subnodes of the component image node */
	for (noffset = fdt_first_subnode(fit, image_noffset);
	     noffset >= 0;
	     noffset = fdt_next_subnode(fit, noffset)) {
		const char *name = fit_get_name(fit, noffset, NULL);

		if (!strncmp(name, FIT_SIG_NODENAME,
			     strlen(FIT_SIG_NODENAME))) {
			ret = fit_image_check_sig(fit, noffset, data,
							size, -1, &err_msg);
			if (ret) {
				puts("- ");
			} else {
				puts("+ ");
				verified = 1;
				break;
			}
		}
	}

	if (noffset == -FDT_ERR_TRUNCATED || noffset == -FDT_ERR_BADSTRUCTURE) {
		err_msg = "Corrupted or truncated tree";
		goto error;
	}

	return verified ? 0 : -EPERM;

error:
	printf(" error!\n%s for '%s' hash node in '%s' image node\n",
	       err_msg, fit_get_name(fit, noffset, NULL),
	       fit_get_name(fit, image_noffset, NULL));
	return -1;
}

int fit_image_verify_required_sigs(const void *fit, int image_noffset,
		const char *data, size_t size, const void *sig_blob,
		int *no_sigsp)
{
	int verify_count = 0;
	int noffset;
	int sig_node;

	/* Work out what we need to verify */
	*no_sigsp = 1;
	sig_node = fdt_subnode_offset(sig_blob, 0, FIT_SIG_NODENAME);
	if (sig_node < 0) {
		debug("%s: No signature node found: %s\n", __func__,
		      fdt_strerror(sig_node));
		return 0;
	}

	for (noffset = fdt_first_subnode(sig_blob, sig_node);
	     noffset >= 0;
	     noffset = fdt_next_subnode(sig_blob, noffset)) {
		const char *required;
		int ret;

		required = fdt_getprop(sig_blob, noffset, "required", NULL);
		if (!required || strcmp(required, "image"))
			continue;
		ret = fit_image_verify_sig(fit, image_noffset, data, size,
					sig_blob, noffset);
		if (ret) {
			printf("Failed to verify required signature '%s'\n",
			       fit_get_name(sig_blob, noffset, NULL));
			return ret;
		}
		verify_count++;
	}

	if (verify_count)
		*no_sigsp = 0;

	return 0;
}

int fit_config_check_sig(const void *fit, int noffset, int required_keynode,
			 char **err_msgp)
{
	char * const exc_prop[] = {"data"};
	const char *prop, *end, *name;
	struct image_sign_info info;
	const uint32_t *strings;
	uint8_t *fit_value;
	int fit_value_len;
	int max_regions;
	int i, prop_len;
	char path[200];
	int count;

	debug("%s: fdt=%p, conf='%s', sig='%s'\n", __func__, gd_fdt_blob(),
	      fit_get_name(fit, noffset, NULL),
	      fit_get_name(gd_fdt_blob(), required_keynode, NULL));
	*err_msgp = NULL;
	if (fit_image_setup_verify(&info, fit, noffset, required_keynode,
				   err_msgp))
		return -1;

	if (fit_image_hash_get_value(fit, noffset, &fit_value,
				     &fit_value_len)) {
		*err_msgp = "Can't get hash value property";
		return -1;
	}

	/* Count the number of strings in the property */
	prop = fdt_getprop(fit, noffset, "hashed-nodes", &prop_len);
	end = prop ? prop + prop_len : prop;
	for (name = prop, count = 0; name < end; name++)
		if (!*name)
			count++;
	if (!count) {
		*err_msgp = "Can't get hashed-nodes property";
		return -1;
	}

	/* Add a sanity check here since we are using the stack */
	if (count > IMAGE_MAX_HASHED_NODES) {
		*err_msgp = "Number of hashed nodes exceeds maximum";
		return -1;
	}

	/* Create a list of node names from those strings */
	char *node_inc[count];

	debug("Hash nodes (%d):\n", count);
	for (name = prop, i = 0; name < end; name += strlen(name) + 1, i++) {
		debug("   '%s'\n", name);
		node_inc[i] = (char *)name;
	}

	/*
	 * Each node can generate one region for each sub-node. Allow for
	 * 7 sub-nodes (hash@1, signature@1, etc.) and some extra.
	 */
	max_regions = 20 + count * 7;
	struct fdt_region fdt_regions[max_regions];

	/* Get a list of regions to hash */
	count = fdt_find_regions(fit, node_inc, count,
			exc_prop, ARRAY_SIZE(exc_prop),
			fdt_regions, max_regions - 1,
			path, sizeof(path), 0);
	if (count < 0) {
		*err_msgp = "Failed to hash configuration";
		return -1;
	}
	if (count == 0) {
		*err_msgp = "No data to hash";
		return -1;
	}
	if (count >= max_regions - 1) {
		*err_msgp = "Too many hash regions";
		return -1;
	}

	/* Add the strings */
	strings = fdt_getprop(fit, noffset, "hashed-strings", NULL);
	if (strings) {
		fdt_regions[count].offset = fdt_off_dt_strings(fit) +
				fdt32_to_cpu(strings[0]);
		fdt_regions[count].size = fdt32_to_cpu(strings[1]);
		count++;
	}

	/* Allocate the region list on the stack */
	struct image_region region[count];

	fit_region_make_list(fit, fdt_regions, count, region);
	if (info.algo->verify(&info, region, count, fit_value,
			      fit_value_len)) {
		*err_msgp = "Verification failed";
		return -1;
	}

	return 0;
}

static int fit_config_verify_sig(const void *fit, int conf_noffset,
		const void *sig_blob, int sig_offset)
{
	int noffset;
	char *err_msg = "";
	int verified = 0;
	int ret;

	/* Process all hash subnodes of the component conf node */
	for (noffset = fdt_first_subnode(fit, conf_noffset);
	     noffset >= 0;
	     noffset = fdt_next_subnode(fit, noffset)) {
		const char *name = fit_get_name(fit, noffset, NULL);

		if (!strncmp(name, FIT_SIG_NODENAME,
			     strlen(FIT_SIG_NODENAME))) {
			ret = fit_config_check_sig(fit, noffset, sig_offset,
						   &err_msg);
			if (ret) {
				puts("- ");
			} else {
				puts("+ ");
				verified = 1;
				break;
			}
		}
	}

	if (noffset == -FDT_ERR_TRUNCATED || noffset == -FDT_ERR_BADSTRUCTURE) {
		err_msg = "Corrupted or truncated tree";
		goto error;
	}

	return verified ? 0 : -EPERM;

error:
	printf(" error!\n%s for '%s' hash node in '%s' config node\n",
	       err_msg, fit_get_name(fit, noffset, NULL),
	       fit_get_name(fit, conf_noffset, NULL));
	return -1;
}

int fit_config_verify_required_sigs(const void *fit, int conf_noffset,
		const void *sig_blob)
{
	int noffset;
	int sig_node;

	/* Work out what we need to verify */
	sig_node = fdt_subnode_offset(sig_blob, 0, FIT_SIG_NODENAME);
	if (sig_node < 0) {
		debug("%s: No signature node found: %s\n", __func__,
		      fdt_strerror(sig_node));
		return 0;
	}

	for (noffset = fdt_first_subnode(sig_blob, sig_node);
	     noffset >= 0;
	     noffset = fdt_next_subnode(sig_blob, noffset)) {
		const char *required;
		int ret;

		required = fdt_getprop(sig_blob, noffset, "required", NULL);
		if (!required || strcmp(required, "conf"))
			continue;
		ret = fit_config_verify_sig(fit, conf_noffset, sig_blob,
					    noffset);
		if (ret) {
			printf("Failed to verify required signature '%s'\n",
			       fit_get_name(sig_blob, noffset, NULL));
			return ret;
		}
	}

	return 0;
}

int fit_config_verify(const void *fit, int conf_noffset)
{
	return fit_config_verify_required_sigs(fit, conf_noffset,
					       gd_fdt_blob());
}
