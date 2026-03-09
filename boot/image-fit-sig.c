// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (c) 2013, Google Inc.
 */

#ifdef USE_HOSTCC
#include "mkimage.h"
#include <time.h>
#else
#include <log.h>
#include <malloc.h>
#include <asm/global_data.h>
DECLARE_GLOBAL_DATA_PTR;
#endif /* !USE_HOSTCC*/
#include <fdt_region.h>
#include <image.h>
#include <u-boot/rsa.h>
#include <u-boot/hash-checksum.h>

#define IMAGE_MAX_HASHED_NODES		100
#define FIT_MAX_HASH_PATH_BUF		4096

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
					  struct fdt_region *fdt_regions,
					  int count,
					  struct image_region *region)
{
	int i;

	debug("Hash regions:\n");
	debug("%10s %10s\n", "Offset", "Size");

	/*
	 * Use malloc() except in SPL (to save code size). In SPL the caller
	 * must allocate the array.
	 */
	if (!IS_ENABLED(CONFIG_XPL_BUILD) && !region)
		region = calloc(sizeof(*region), count);
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
				  const void *fit, int noffset,
				  const void *key_blob, int required_keynode,
				  char **err_msgp)
{
	const char *algo_name;
	const char *padding_name;

	if (fdt_totalsize(fit) > CONFIG_VAL(FIT_SIGNATURE_MAX_SIZE)) {
		*err_msgp = "Total size too large";
		return 1;
	}
	if (fit_image_hash_get_algo(fit, noffset, &algo_name)) {
		*err_msgp = "Can't get hash algo property";
		return -1;
	}

	padding_name = fdt_getprop(fit, noffset, "padding", NULL);
	if (!padding_name)
		padding_name = RSA_DEFAULT_PADDING_NAME;

	memset(info, '\0', sizeof(*info));
	info->keyname = fdt_getprop(fit, noffset, FIT_KEY_HINT, NULL);
	info->fit = fit;
	info->node_offset = noffset;
	info->name = algo_name;
	info->checksum = image_get_checksum_algo(algo_name);
	info->crypto = image_get_crypto_algo(algo_name);
	info->padding = image_get_padding_algo(padding_name);
	info->fdt_blob = key_blob;
	info->required_keynode = required_keynode;
	printf("%s:%s", algo_name, info->keyname);

	if (!info->checksum || !info->crypto) {
		*err_msgp = "Unknown signature algorithm";
		return -1;
	}

	return 0;
}

int fit_image_check_sig(const void *fit, int noffset, const void *data,
			size_t size, const void *key_blob, int required_keynode,
			char **err_msgp)
{
	struct image_sign_info info;
	struct image_region region;
	uint8_t *fit_value;
	int fit_value_len;

	*err_msgp = NULL;
	if (fit_image_setup_verify(&info, fit, noffset, key_blob,
				   required_keynode, err_msgp))
		return -1;

	if (fit_image_hash_get_value(fit, noffset, &fit_value,
				     &fit_value_len)) {
		*err_msgp = "Can't get hash value property";
		return -1;
	}

	region.data = data;
	region.size = size;

	if (info.crypto->verify(&info, &region, 1, fit_value, fit_value_len)) {
		*err_msgp = "Verification failed";
		return -1;
	}

	return 0;
}

static int fit_image_verify_sig(const void *fit, int image_noffset,
				const char *data, size_t size,
				const void *key_blob, int key_offset)
{
	int noffset;
	char *err_msg = "";
	int verified = 0;
	int ret;

	/* Process all hash subnodes of the component image node */
	fdt_for_each_subnode(noffset, fit, image_noffset) {
		const char *name = fit_get_name(fit, noffset, NULL);

		/*
		 * We don't support this since libfdt considers names with the
		 * name root but different @ suffix to be equal
		 */
		if (strchr(name, '@')) {
			err_msg = "Node name contains @";
			goto error;
		}
		if (!strncmp(name, FIT_SIG_NODENAME,
			     strlen(FIT_SIG_NODENAME))) {
			ret = fit_image_check_sig(fit, noffset, data, size,
						  key_blob, -1, &err_msg);
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
				   const char *data, size_t size,
				   const void *key_blob, int *no_sigsp)
{
	int verify_count = 0;
	int noffset;
	int key_node;

#ifdef USE_HOSTCC
	if (!key_blob)
		return 0;
#endif

	/* Work out what we need to verify */
	*no_sigsp = 1;
	key_node = fdt_subnode_offset(key_blob, 0, FIT_SIG_NODENAME);
	if (key_node < 0) {
		debug("%s: No signature node found: %s\n", __func__,
		      fdt_strerror(key_node));
		return 0;
	}

	fdt_for_each_subnode(noffset, key_blob, key_node) {
		const char *required;
		int ret;

		required = fdt_getprop(key_blob, noffset, FIT_KEY_REQUIRED,
				       NULL);
		if (!required || strcmp(required, "image"))
			continue;
		ret = fit_image_verify_sig(fit, image_noffset, data, size,
					   key_blob, noffset);
		if (ret) {
			printf("Failed to verify required signature '%s'\n",
			       fit_get_name(key_blob, noffset, NULL));
			return ret;
		}
		verify_count++;
	}

	if (verify_count)
		*no_sigsp = 0;

	return 0;
}

/**
 * fit_config_add_hash() - Add hash nodes for one image to the node list
 *
 * Adds the image path, all its hash-* subnode paths, and its cipher
 * subnode path (if present) to the packed buffer.
 *
 * @fit:		FIT blob
 * @image_noffset:	Image node offset (e.g. /images/kernel-1)
 * @node_inc:		Array of path pointers to fill
 * @count:		Pointer to current count (updated on return)
 * @max_nodes:		Maximum entries in @node_inc
 * @buf:		Buffer for packed path strings
 * @buf_used:		Pointer to bytes used in @buf (updated on return)
 * @buf_len:		Total size of @buf
 * Return: 0 on success, -ve on error
 */
static int fit_config_add_hash(const void *fit, int image_noffset,
			       char **node_inc, int *count, int max_nodes,
			       char *buf, int *buf_used, int buf_len)
{
	int noffset, hash_count, ret, len;

	if (*count >= max_nodes)
		return -ENOSPC;

	ret = fdt_get_path(fit, image_noffset, buf + *buf_used,
			   buf_len - *buf_used);
	if (ret < 0)
		return -ENOENT;
	len = strlen(buf + *buf_used) + 1;
	node_inc[(*count)++] = buf + *buf_used;
	*buf_used += len;

	/* Add all this image's hash subnodes */
	hash_count = 0;
	for (noffset = fdt_first_subnode(fit, image_noffset);
	     noffset >= 0;
	     noffset = fdt_next_subnode(fit, noffset)) {
		const char *name = fit_get_name(fit, noffset, NULL);

		if (strncmp(name, FIT_HASH_NODENAME,
			    strlen(FIT_HASH_NODENAME)))
			continue;
		if (*count >= max_nodes)
			return -ENOSPC;
		ret = fdt_get_path(fit, noffset, buf + *buf_used,
				   buf_len - *buf_used);
		if (ret < 0)
			return -ENOENT;
		len = strlen(buf + *buf_used) + 1;
		node_inc[(*count)++] = buf + *buf_used;
		*buf_used += len;
		hash_count++;
	}

	if (!hash_count) {
		printf("No hash nodes in image '%s'\n",
		       fdt_get_name(fit, image_noffset, NULL));
		return -ENOMSG;
	}

	/* Add this image's cipher node if present */
	noffset = fdt_subnode_offset(fit, image_noffset, FIT_CIPHER_NODENAME);
	if (noffset != -FDT_ERR_NOTFOUND) {
		if (noffset < 0)
			return -EIO;
		if (*count >= max_nodes)
			return -ENOSPC;
		ret = fdt_get_path(fit, noffset, buf + *buf_used,
				   buf_len - *buf_used);
		if (ret < 0)
			return -ENOENT;
		len = strlen(buf + *buf_used) + 1;
		node_inc[(*count)++] = buf + *buf_used;
		*buf_used += len;
	}

	return 0;
}

/**
 * fit_config_get_hash_list() - Build the list of nodes to hash
 *
 * Works through every image referenced by the configuration and collects the
 * node paths: root + config + all referenced images with their hash and
 * cipher subnodes.
 *
 * Properties known not to be image references (description, compatible,
 * default, load-only) are skipped, so any new image type is covered by default.
 *
 * @fit:	FIT blob
 * @conf_noffset: Configuration node offset
 * @node_inc:	Array to fill with path string pointers
 * @max_nodes:	Size of @node_inc array
 * @buf:	Buffer for packed null-terminated path strings
 * @buf_len:	Size of @buf
 * Return: number of entries in @node_inc, or -ve on error
 */
static int fit_config_get_hash_list(const void *fit, int conf_noffset,
				    char **node_inc, int max_nodes,
				    char *buf, int buf_len)
{
	const char *conf_name;
	int image_count;
	int prop_offset;
	int used = 0;
	int count = 0;
	int ret, len;

	conf_name = fit_get_name(fit, conf_noffset, NULL);

	/* Always include the root node and the configuration node */
	if (max_nodes < 2)
		return -ENOSPC;

	len = 2;  /* "/" + nul */
	if (len > buf_len)
		return -ENOSPC;
	strcpy(buf, "/");
	node_inc[count++] = buf;
	used += len;

	len = snprintf(buf + used, buf_len - used, "%s/%s", FIT_CONFS_PATH,
		       conf_name) + 1;
	if (used + len > buf_len)
		return -ENOSPC;
	node_inc[count++] = buf + used;
	used += len;

	/* Process each image referenced by the config */
	image_count = 0;
	fdt_for_each_property_offset(prop_offset, fit, conf_noffset) {
		const char *prop_name;
		int img_count, i;

		fdt_getprop_by_offset(fit, prop_offset, &prop_name, NULL);
		if (!prop_name)
			continue;

		/* Skip properties that are not image references */
		if (!strcmp(prop_name, FIT_DESC_PROP) ||
		    !strcmp(prop_name, FIT_COMPAT_PROP) ||
		    !strcmp(prop_name, FIT_DEFAULT_PROP))
			continue;

		img_count = fdt_stringlist_count(fit, conf_noffset, prop_name);
		for (i = 0; i < img_count; i++) {
			int noffset;

			noffset = fit_conf_get_prop_node_index(fit,
							       conf_noffset,
							       prop_name, i);
			if (noffset < 0)
				continue;

			ret = fit_config_add_hash(fit, noffset, node_inc,
						  &count, max_nodes, buf, &used,
						  buf_len);
			if (ret < 0)
				return ret;

			image_count++;
		}
	}

	if (!image_count) {
		printf("No images in config '%s'\n", conf_name);
		return -ENOMSG;
	}

	return count;
}

/**
 * fit_config_check_sig() - Check the signature of a config
 *
 * Here we are looking at a particular signature that needs verification (here
 * signature-1):
 *
 *	configurations {
 *		default = "conf-1";
 *		conf-1 {
 *			kernel = "kernel-1";
 *			fdt = "fdt-1";
 *			signature-1 {
 *				algo = "sha1,rsa2048";
 *				value = <...conf 1 signature...>;
 *			};
 *		};
 *
 * @fit: FIT to check
 * @noffset: Offset of the signature node being checked (e.g.
 *	 /configurations/conf-1/signature-1)
 * @conf_noffset: Offset of configuration node (e.g. /configurations/conf-1)
 * @key_blob: Blob containing the keys to check against
 * @required_keynode:	Offset in @key_blob of the required key node,
 *			if any. If this is given, then the configuration wil not
 *			pass verification unless that key is used. If this is
 *			-1 then any signature will do.
 * @err_msgp:		In the event of an error, this will be pointed to a
 *			help error string to display to the user.
 * Return: 0 if all verified ok, <0 on error
 */
static int fit_config_check_sig(const void *fit, int noffset, int conf_noffset,
				const void *key_blob, int required_keynode,
				char **err_msgp)
{
	static char * const exc_prop[] = {
		FIT_DATA_PROP,
		FIT_DATA_SIZE_PROP,
		FIT_DATA_POSITION_PROP,
		FIT_DATA_OFFSET_PROP,
	};
	char *node_inc[IMAGE_MAX_HASHED_NODES];
	char hash_buf[FIT_MAX_HASH_PATH_BUF];
	struct image_sign_info info;
	const uint32_t *strings;
	uint8_t *fit_value;
	int fit_value_len;
	int max_regions;
	char path[200];
	int count;

	debug("%s: fdt=%p, conf='%s', sig='%s'\n", __func__, key_blob,
	      fit_get_name(fit, noffset, NULL),
	      fit_get_name(key_blob, required_keynode, NULL));
	*err_msgp = NULL;
	if (fit_image_setup_verify(&info, fit, noffset, key_blob,
				   required_keynode, err_msgp))
		return -1;

	if (fit_image_hash_get_value(fit, noffset, &fit_value,
				     &fit_value_len)) {
		*err_msgp = "Can't get hash value property";
		return -1;
	}

	/* Build the node list from the config, ignoring hashed-nodes */
	count = fit_config_get_hash_list(fit, conf_noffset,
					 node_inc, IMAGE_MAX_HASHED_NODES,
					 hash_buf, sizeof(hash_buf));
	if (count < 0) {
		*err_msgp = "Failed to build hash node list";
		return -1;
	}

	/*
	 * Each node can generate one region for each sub-node. Allow for
	 * 7 sub-nodes (hash-1, signature-1, etc.) and some extra.
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
		/*
		 * The strings region offset must be a static 0x0.
		 * This is set in tool/image-host.c
		 */
		fdt_regions[count].offset = fdt_off_dt_strings(fit);
		fdt_regions[count].size = fdt32_to_cpu(strings[1]);
		count++;
	}

	/* Allocate the region list on the stack */
	struct image_region region[count];

	fit_region_make_list(fit, fdt_regions, count, region);
	if (info.crypto->verify(&info, region, count, fit_value,
				fit_value_len)) {
		*err_msgp = "Verification failed";
		return -1;
	}

	return 0;
}

/**
 * fit_config_verify_key() - Verify that a configuration is signed with a key
 *
 * Here we are looking at a particular configuration that needs verification:
 *
 *	configurations {
 *		default = "conf-1";
 *		conf-1 {
 *			kernel = "kernel-1";
 *			fdt = "fdt-1";
 *			signature-1 {
 *				algo = "sha1,rsa2048";
 *				value = <...conf 1 signature...>;
 *			};
 *		};
 *
 * We must check each of the signature subnodes of conf-1. Hopefully one of them
 * will match the key at key_offset.
 *
 * @fit: FIT to check
 * @conf_noffset: Offset of the configuration node to check (e.g.
 *	/configurations/conf-1)
 * @key_blob: Blob containing the keys to check against
 * @key_offset: Offset of the key to check within @key_blob
 * @return 0 if OK, -EPERM if any signatures did not verify, or the
 *	configuration node has an invalid name
 */
static int fit_config_verify_key(const void *fit, int conf_noffset,
				 const void *key_blob, int key_offset)
{
	int noffset;
	char *err_msg = "No 'signature' subnode found";
	int verified = 0;
	int ret;

	/* Process all hash subnodes of the component conf node */
	fdt_for_each_subnode(noffset, fit, conf_noffset) {
		const char *name = fit_get_name(fit, noffset, NULL);

		if (!strncmp(name, FIT_SIG_NODENAME,
			     strlen(FIT_SIG_NODENAME))) {
			ret = fit_config_check_sig(fit, noffset, conf_noffset,
						   key_blob, key_offset,
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

	if (verified)
		return 0;

error:
	printf(" error!\n%s for '%s' hash node in '%s' config node\n",
	       err_msg, fit_get_name(fit, noffset, NULL),
	       fit_get_name(fit, conf_noffset, NULL));
	return -EPERM;
}

/**
 * fit_config_verify_required_keys() - verify any required signatures for config
 *
 * This looks through all the signatures we expect and verifies that at least
 * all the required ones are valid signatures for the configuration
 *
 * @fit: FIT to check
 * @conf_noffset: Offset of the configuration node to check (e.g.
 *	/configurations/conf-1)
 * @key_blob: Blob containing the keys to check against
 * @return 0 if OK, -EPERM if any signatures did not verify, or the
 *	configuration node has an invalid name
 */
static int fit_config_verify_required_keys(const void *fit, int conf_noffset,
					   const void *key_blob)
{
	const char *name = fit_get_name(fit, conf_noffset, NULL);
	int noffset;
	int key_node;
	int verified = 0;
	int reqd_sigs = 0;
	bool reqd_policy_all = true;
	const char *reqd_mode;

#ifdef USE_HOSTCC
	if (!key_blob)
		return 0;
#endif

	/*
	 * We don't support this since libfdt considers names with the
	 * name root but different @ suffix to be equal
	 */
	if (strchr(name, '@')) {
		printf("Configuration node '%s' contains '@'\n", name);
		return -EPERM;
	}

	/* Work out what we need to verify */
	key_node = fdt_subnode_offset(key_blob, 0, FIT_SIG_NODENAME);
	if (key_node < 0) {
		debug("%s: No signature node found: %s\n", __func__,
		      fdt_strerror(key_node));
		return 0;
	}

	/* Get required-mode policy property from DTB */
	reqd_mode = fdt_getprop(key_blob, key_node, "required-mode", NULL);
	if (reqd_mode && !strcmp(reqd_mode, "any"))
		reqd_policy_all = false;

	debug("%s: required-mode policy set to '%s'\n", __func__,
	      reqd_policy_all ? "all" : "any");

	/*
	 * The algorithm here is a little convoluted due to how we want it to
	 * work. Here we work through each of the signature nodes in the
	 * public-key area. These are in the U-Boot control devicetree. Each
	 * node was created by signing a configuration, so we check if it is
	 * 'required' and if so, request that it be verified.
	 */
	fdt_for_each_subnode(noffset, key_blob, key_node) {
		const char *required;
		int ret;

		required = fdt_getprop(key_blob, noffset, FIT_KEY_REQUIRED,
				       NULL);
		if (!required || strcmp(required, "conf"))
			continue;

		reqd_sigs++;

		ret = fit_config_verify_key(fit, conf_noffset, key_blob,
					    noffset);
		if (ret) {
			if (reqd_policy_all) {
				printf("Failed to verify required signature '%s'\n",
				       fit_get_name(key_blob, noffset, NULL));
				return ret;
			}
		} else {
			verified++;
			if (!reqd_policy_all)
				break;
		}
	}

	if (reqd_sigs && !verified) {
		printf("Failed to verify 'any' of the required signature(s)\n");
		return -EPERM;
	}

	return 0;
}

int fit_config_verify(const void *fit, int conf_noffset)
{
	return fit_config_verify_required_keys(fit, conf_noffset,
					       gd_fdt_blob());
}
