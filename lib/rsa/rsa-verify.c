/*
 * Copyright (c) 2013, Google Inc.
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#ifndef USE_HOSTCC
#include <common.h>
#include <fdtdec.h>
#include <asm/types.h>
#include <asm/byteorder.h>
#include <linux/errno.h>
#include <asm/types.h>
#include <asm/unaligned.h>
#include <dm.h>
#else
#include "fdt_host.h"
#include "mkimage.h"
#include <fdt_support.h>
#endif
#include <u-boot/rsa-mod-exp.h>
#include <u-boot/rsa.h>

/* Default public exponent for backward compatibility */
#define RSA_DEFAULT_PUBEXP	65537

/**
 * rsa_verify_padding() - Verify RSA message padding is valid
 *
 * Verify a RSA message's padding is consistent with PKCS1.5
 * padding as described in the RSA PKCS#1 v2.1 standard.
 *
 * @msg:	Padded message
 * @pad_len:	Number of expected padding bytes
 * @algo:	Checksum algo structure having information on DER encoding etc.
 * @return 0 on success, != 0 on failure
 */
static int rsa_verify_padding(const uint8_t *msg, const int pad_len,
			      struct checksum_algo *algo)
{
	int ff_len;
	int ret;

	/* first byte must be 0x00 */
	ret = *msg++;
	/* second byte must be 0x01 */
	ret |= *msg++ ^ 0x01;
	/* next ff_len bytes must be 0xff */
	ff_len = pad_len - algo->der_len - 3;
	ret |= *msg ^ 0xff;
	ret |= memcmp(msg, msg+1, ff_len-1);
	msg += ff_len;
	/* next byte must be 0x00 */
	ret |= *msg++;
	/* next der_len bytes must match der_prefix */
	ret |= memcmp(msg, algo->der_prefix, algo->der_len);

	return ret;
}

/**
 * rsa_verify_key() - Verify a signature against some data using RSA Key
 *
 * Verify a RSA PKCS1.5 signature against an expected hash using
 * the RSA Key properties in prop structure.
 *
 * @prop:	Specifies key
 * @sig:	Signature
 * @sig_len:	Number of bytes in signature
 * @hash:	Pointer to the expected hash
 * @key_len:	Number of bytes in rsa key
 * @algo:	Checksum algo structure having information on DER encoding etc.
 * @return 0 if verified, -ve on error
 */
static int rsa_verify_key(struct key_prop *prop, const uint8_t *sig,
			  const uint32_t sig_len, const uint8_t *hash,
			  const uint32_t key_len, struct checksum_algo *algo)
{
	int pad_len;
	int ret;
#if !defined(USE_HOSTCC)
	struct udevice *mod_exp_dev;
#endif

	if (!prop || !sig || !hash || !algo)
		return -EIO;

	if (sig_len != (prop->num_bits / 8)) {
		debug("Signature is of incorrect length %d\n", sig_len);
		return -EINVAL;
	}

	debug("Checksum algorithm: %s", algo->name);

	/* Sanity check for stack size */
	if (sig_len > RSA_MAX_SIG_BITS / 8) {
		debug("Signature length %u exceeds maximum %d\n", sig_len,
		      RSA_MAX_SIG_BITS / 8);
		return -EINVAL;
	}

	uint8_t buf[sig_len];

#if !defined(USE_HOSTCC)
	ret = uclass_get_device(UCLASS_MOD_EXP, 0, &mod_exp_dev);
	if (ret) {
		printf("RSA: Can't find Modular Exp implementation\n");
		return -EINVAL;
	}

	ret = rsa_mod_exp(mod_exp_dev, sig, sig_len, prop, buf);
#else
	ret = rsa_mod_exp_sw(sig, sig_len, prop, buf);
#endif
	if (ret) {
		debug("Error in Modular exponentation\n");
		return ret;
	}

	pad_len = key_len - algo->checksum_len;

	/* Check pkcs1.5 padding bytes. */
	ret = rsa_verify_padding(buf, pad_len, algo);
	if (ret) {
		debug("In RSAVerify(): Padding check failed!\n");
		return -EINVAL;
	}

	/* Check hash. */
	if (memcmp((uint8_t *)buf + pad_len, hash, sig_len - pad_len)) {
		debug("In RSAVerify(): Hash check failed!\n");
		return -EACCES;
	}

	return 0;
}

/**
 * rsa_verify_with_keynode() - Verify a signature against some data using
 * information in node with prperties of RSA Key like modulus, exponent etc.
 *
 * Parse sign-node and fill a key_prop structure with properties of the
 * key.  Verify a RSA PKCS1.5 signature against an expected hash using
 * the properties parsed
 *
 * @info:	Specifies key and FIT information
 * @hash:	Pointer to the expected hash
 * @sig:	Signature
 * @sig_len:	Number of bytes in signature
 * @node:	Node having the RSA Key properties
 * @return 0 if verified, -ve on error
 */
static int rsa_verify_with_keynode(struct image_sign_info *info,
				   const void *hash, uint8_t *sig,
				   uint sig_len, int node)
{
	const void *blob = info->fdt_blob;
	struct key_prop prop;
	int length;
	int ret = 0;

	if (node < 0) {
		debug("%s: Skipping invalid node", __func__);
		return -EBADF;
	}

	prop.num_bits = fdtdec_get_int(blob, node, "rsa,num-bits", 0);

	prop.n0inv = fdtdec_get_int(blob, node, "rsa,n0-inverse", 0);

	prop.public_exponent = fdt_getprop(blob, node, "rsa,exponent", &length);
	if (!prop.public_exponent || length < sizeof(uint64_t))
		prop.public_exponent = NULL;

	prop.exp_len = sizeof(uint64_t);

	prop.modulus = fdt_getprop(blob, node, "rsa,modulus", NULL);

	prop.rr = fdt_getprop(blob, node, "rsa,r-squared", NULL);

	if (!prop.num_bits || !prop.modulus) {
		debug("%s: Missing RSA key info", __func__);
		return -EFAULT;
	}

	ret = rsa_verify_key(&prop, sig, sig_len, hash,
			     info->crypto->key_len, info->checksum);

	return ret;
}

int rsa_verify(struct image_sign_info *info,
	       const struct image_region region[], int region_count,
	       uint8_t *sig, uint sig_len)
{
	const void *blob = info->fdt_blob;
	/* Reserve memory for maximum checksum-length */
	uint8_t hash[info->crypto->key_len];
	int ndepth, noffset;
	int sig_node, node;
	char name[100];
	int ret;

	/*
	 * Verify that the checksum-length does not exceed the
	 * rsa-signature-length
	 */
	if (info->checksum->checksum_len >
	    info->crypto->key_len) {
		debug("%s: invlaid checksum-algorithm %s for %s\n",
		      __func__, info->checksum->name, info->crypto->name);
		return -EINVAL;
	}

	sig_node = fdt_subnode_offset(blob, 0, FIT_SIG_NODENAME);
	if (sig_node < 0) {
		debug("%s: No signature node found\n", __func__);
		return -ENOENT;
	}

	/* Calculate checksum with checksum-algorithm */
	ret = info->checksum->calculate(info->checksum->name,
					region, region_count, hash);
	if (ret < 0) {
		debug("%s: Error in checksum calculation\n", __func__);
		return -EINVAL;
	}

	/* See if we must use a particular key */
	if (info->required_keynode != -1) {
		ret = rsa_verify_with_keynode(info, hash, sig, sig_len,
			info->required_keynode);
		if (!ret)
			return ret;
	}

	/* Look for a key that matches our hint */
	snprintf(name, sizeof(name), "key-%s", info->keyname);
	node = fdt_subnode_offset(blob, sig_node, name);
	ret = rsa_verify_with_keynode(info, hash, sig, sig_len, node);
	if (!ret)
		return ret;

	/* No luck, so try each of the keys in turn */
	for (ndepth = 0, noffset = fdt_next_node(info->fit, sig_node, &ndepth);
			(noffset >= 0) && (ndepth > 0);
			noffset = fdt_next_node(info->fit, noffset, &ndepth)) {
		if (ndepth == 1 && noffset != node) {
			ret = rsa_verify_with_keynode(info, hash, sig, sig_len,
						      noffset);
			if (!ret)
				break;
		}
	}

	return ret;
}
