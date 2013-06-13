/*
 * Copyright (c) 2013, Google Inc.
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

#ifdef USE_HOSTCC
#include "mkimage.h"
#include <time.h>
#else
#include <common.h>
#include <malloc.h>
DECLARE_GLOBAL_DATA_PTR;
#endif /* !USE_HOSTCC*/
#include <errno.h>
#include <image.h>

struct image_sig_algo image_sig_algos[] = {
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
