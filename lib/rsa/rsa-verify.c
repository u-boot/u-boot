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
#include <asm/errno.h>
#include <asm/types.h>
#include <asm/unaligned.h>
#else
#include "fdt_host.h"
#include "mkimage.h"
#include <fdt_support.h>
#endif
#include <rsa.h>
#include <sha1.h>
#include <sha256.h>

#define UINT64_MULT32(v, multby)  (((uint64_t)(v)) * ((uint32_t)(multby)))

#define get_unaligned_be32(a) fdt32_to_cpu(*(uint32_t *)a)
#define put_unaligned_be32(a, b) (*(uint32_t *)(b) = cpu_to_fdt32(a))

/**
 * subtract_modulus() - subtract modulus from the given value
 *
 * @key:	Key containing modulus to subtract
 * @num:	Number to subtract modulus from, as little endian word array
 */
static void subtract_modulus(const struct rsa_public_key *key, uint32_t num[])
{
	int64_t acc = 0;
	uint i;

	for (i = 0; i < key->len; i++) {
		acc += (uint64_t)num[i] - key->modulus[i];
		num[i] = (uint32_t)acc;
		acc >>= 32;
	}
}

/**
 * greater_equal_modulus() - check if a value is >= modulus
 *
 * @key:	Key containing modulus to check
 * @num:	Number to check against modulus, as little endian word array
 * @return 0 if num < modulus, 1 if num >= modulus
 */
static int greater_equal_modulus(const struct rsa_public_key *key,
				 uint32_t num[])
{
	uint32_t i;

	for (i = key->len - 1; i >= 0; i--) {
		if (num[i] < key->modulus[i])
			return 0;
		if (num[i] > key->modulus[i])
			return 1;
	}

	return 1;  /* equal */
}

/**
 * montgomery_mul_add_step() - Perform montgomery multiply-add step
 *
 * Operation: montgomery result[] += a * b[] / n0inv % modulus
 *
 * @key:	RSA key
 * @result:	Place to put result, as little endian word array
 * @a:		Multiplier
 * @b:		Multiplicand, as little endian word array
 */
static void montgomery_mul_add_step(const struct rsa_public_key *key,
		uint32_t result[], const uint32_t a, const uint32_t b[])
{
	uint64_t acc_a, acc_b;
	uint32_t d0;
	uint i;

	acc_a = (uint64_t)a * b[0] + result[0];
	d0 = (uint32_t)acc_a * key->n0inv;
	acc_b = (uint64_t)d0 * key->modulus[0] + (uint32_t)acc_a;
	for (i = 1; i < key->len; i++) {
		acc_a = (acc_a >> 32) + (uint64_t)a * b[i] + result[i];
		acc_b = (acc_b >> 32) + (uint64_t)d0 * key->modulus[i] +
				(uint32_t)acc_a;
		result[i - 1] = (uint32_t)acc_b;
	}

	acc_a = (acc_a >> 32) + (acc_b >> 32);

	result[i - 1] = (uint32_t)acc_a;

	if (acc_a >> 32)
		subtract_modulus(key, result);
}

/**
 * montgomery_mul() - Perform montgomery mutitply
 *
 * Operation: montgomery result[] = a[] * b[] / n0inv % modulus
 *
 * @key:	RSA key
 * @result:	Place to put result, as little endian word array
 * @a:		Multiplier, as little endian word array
 * @b:		Multiplicand, as little endian word array
 */
static void montgomery_mul(const struct rsa_public_key *key,
		uint32_t result[], uint32_t a[], const uint32_t b[])
{
	uint i;

	for (i = 0; i < key->len; ++i)
		result[i] = 0;
	for (i = 0; i < key->len; ++i)
		montgomery_mul_add_step(key, result, a[i], b);
}

/**
 * pow_mod() - in-place public exponentiation
 *
 * @key:	RSA key
 * @inout:	Big-endian word array containing value and result
 */
static int pow_mod(const struct rsa_public_key *key, uint32_t *inout)
{
	uint32_t *result, *ptr;
	uint i;

	/* Sanity check for stack size - key->len is in 32-bit words */
	if (key->len > RSA_MAX_KEY_BITS / 32) {
		debug("RSA key words %u exceeds maximum %d\n", key->len,
		      RSA_MAX_KEY_BITS / 32);
		return -EINVAL;
	}

	uint32_t val[key->len], acc[key->len], tmp[key->len];
	result = tmp;  /* Re-use location. */

	/* Convert from big endian byte array to little endian word array. */
	for (i = 0, ptr = inout + key->len - 1; i < key->len; i++, ptr--)
		val[i] = get_unaligned_be32(ptr);

	montgomery_mul(key, acc, val, key->rr);  /* axx = a * RR / R mod M */
	for (i = 0; i < 16; i += 2) {
		montgomery_mul(key, tmp, acc, acc); /* tmp = acc^2 / R mod M */
		montgomery_mul(key, acc, tmp, tmp); /* acc = tmp^2 / R mod M */
	}
	montgomery_mul(key, result, acc, val);  /* result = XX * a / R mod M */

	/* Make sure result < mod; result is at most 1x mod too large. */
	if (greater_equal_modulus(key, result))
		subtract_modulus(key, result);

	/* Convert to bigendian byte array */
	for (i = key->len - 1, ptr = inout; (int)i >= 0; i--, ptr++)
		put_unaligned_be32(result[i], ptr);
	return 0;
}

static int rsa_verify_key(const struct rsa_public_key *key, const uint8_t *sig,
			  const uint32_t sig_len, const uint8_t *hash,
			  struct checksum_algo *algo)
{
	const uint8_t *padding;
	int pad_len;
	int ret;

	if (!key || !sig || !hash || !algo)
		return -EIO;

	if (sig_len != (key->len * sizeof(uint32_t))) {
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

	uint32_t buf[sig_len / sizeof(uint32_t)];

	memcpy(buf, sig, sig_len);

	ret = pow_mod(key, buf);
	if (ret)
		return ret;

	padding = algo->rsa_padding;
	pad_len = algo->pad_len - algo->checksum_len;

	/* Check pkcs1.5 padding bytes. */
	if (memcmp(buf, padding, pad_len)) {
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

static void rsa_convert_big_endian(uint32_t *dst, const uint32_t *src, int len)
{
	int i;

	for (i = 0; i < len; i++)
		dst[i] = fdt32_to_cpu(src[len - 1 - i]);
}

static int rsa_verify_with_keynode(struct image_sign_info *info,
		const void *hash, uint8_t *sig, uint sig_len, int node)
{
	const void *blob = info->fdt_blob;
	struct rsa_public_key key;
	const void *modulus, *rr;
	int ret;

	if (node < 0) {
		debug("%s: Skipping invalid node", __func__);
		return -EBADF;
	}
	if (!fdt_getprop(blob, node, "rsa,n0-inverse", NULL)) {
		debug("%s: Missing rsa,n0-inverse", __func__);
		return -EFAULT;
	}
	key.len = fdtdec_get_int(blob, node, "rsa,num-bits", 0);
	key.n0inv = fdtdec_get_int(blob, node, "rsa,n0-inverse", 0);
	modulus = fdt_getprop(blob, node, "rsa,modulus", NULL);
	rr = fdt_getprop(blob, node, "rsa,r-squared", NULL);
	if (!key.len || !modulus || !rr) {
		debug("%s: Missing RSA key info", __func__);
		return -EFAULT;
	}

	/* Sanity check for stack size */
	if (key.len > RSA_MAX_KEY_BITS || key.len < RSA_MIN_KEY_BITS) {
		debug("RSA key bits %u outside allowed range %d..%d\n",
		      key.len, RSA_MIN_KEY_BITS, RSA_MAX_KEY_BITS);
		return -EFAULT;
	}
	key.len /= sizeof(uint32_t) * 8;
	uint32_t key1[key.len], key2[key.len];

	key.modulus = key1;
	key.rr = key2;
	rsa_convert_big_endian(key.modulus, modulus, key.len);
	rsa_convert_big_endian(key.rr, rr, key.len);
	if (!key.modulus || !key.rr) {
		debug("%s: Out of memory", __func__);
		return -ENOMEM;
	}

	debug("key length %d\n", key.len);
	ret = rsa_verify_key(&key, sig, sig_len, hash, info->algo->checksum);
	if (ret) {
		printf("%s: RSA failed to verify: %d\n", __func__, ret);
		return ret;
	}

	return 0;
}

int rsa_verify(struct image_sign_info *info,
	       const struct image_region region[], int region_count,
	       uint8_t *sig, uint sig_len)
{
	const void *blob = info->fdt_blob;
	/* Reserve memory for maximum checksum-length */
	uint8_t hash[info->algo->checksum->pad_len];
	int ndepth, noffset;
	int sig_node, node;
	char name[100];
	int ret;

	/*
	 * Verify that the checksum-length does not exceed the
	 * rsa-signature-length
	 */
	if (info->algo->checksum->checksum_len >
	    info->algo->checksum->pad_len) {
		debug("%s: invlaid checksum-algorithm %s for %s\n",
		      __func__, info->algo->checksum->name, info->algo->name);
		return -EINVAL;
	}

	sig_node = fdt_subnode_offset(blob, 0, FIT_SIG_NODENAME);
	if (sig_node < 0) {
		debug("%s: No signature node found\n", __func__);
		return -ENOENT;
	}

	/* Calculate checksum with checksum-algorithm */
	info->algo->checksum->calculate(region, region_count, hash);

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
