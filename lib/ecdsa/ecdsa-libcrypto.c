// SPDX-License-Identifier: GPL-2.0+
/*
 * ECDSA image signing implementation using libcrypto backend
 *
 * The signature is a binary representation of the (R, S) points, padded to the
 * key size. The signature will be (2 * key_size_bits) / 8 bytes.
 *
 * Deviations from behavior of RSA equivalent:
 *  - Verification uses private key. This is not technically required, but a
 *    limitation on how clumsy the openssl API is to use.
 *  - Handling of keys and key paths:
 *    - The '-K' key directory option must contain path to the key file,
 *      instead of the key directory.
 *    - No assumptions are made about the file extension of the key
 *    - The 'key-name-hint' property is only used for naming devicetree nodes,
 *      but is not used for looking up keys on the filesystem.
 *
 * Copyright (c) 2020,2021, Alexandru Gagniuc <mr.nuke.me@gmail.com>
 */

#define OPENSSL_API_COMPAT 0x10101000L

#include <u-boot/ecdsa.h>
#include <u-boot/fdt-libcrypto.h>
#include <openssl/ssl.h>
#include <openssl/ec.h>
#include <openssl/bn.h>

/* Image signing context for openssl-libcrypto */
struct signer {
	EVP_PKEY *evp_key;	/* Pointer to EVP_PKEY object */
	EC_KEY *ecdsa_key;	/* Pointer to EC_KEY object */
	void *hash;		/* Pointer to hash used for verification */
	void *signature;	/* Pointer to output signature. Do not free()!*/
};

struct ecdsa_public_key {
	const char *curve_name;
	const uint8_t *x;
	const uint8_t *y;
	int size_bits;
};

static int fdt_get_key(struct ecdsa_public_key *key, const void *fdt, int node)
{
	int x_len;
	int y_len;

	key->curve_name = fdt_getprop(fdt, node, "ecdsa,curve", NULL);
	if (!key->curve_name)
		return -ENOMSG;

	if (!strcmp(key->curve_name, "prime256v1"))
		key->size_bits = 256;
	else if (!strcmp(key->curve_name, "secp384r1"))
		key->size_bits = 384;
	else
		return -EINVAL;

	key->x = fdt_getprop(fdt, node, "ecdsa,x-point", &x_len);
	key->y = fdt_getprop(fdt, node, "ecdsa,y-point", &y_len);

	if (!key->x || !key->y)
		return -EINVAL;

	if (x_len != key->size_bits / 8 || y_len != key->size_bits / 8)
		return -EINVAL;

	return 0;
}

static int read_key_from_fdt(struct signer *ctx, const void *fdt, int node)
{
	struct ecdsa_public_key pubkey;
	const EC_GROUP *group;
	EC_POINT *point;
	EC_KEY *ec_key;
	int ret;
	int nid;
	int len;

	ret = fdt_get_key(&pubkey, fdt, node);
	if (ret) {
		fprintf(stderr, "Failed to parse ECDSA key from FDT node %d (ret=%d)\n", node, ret);
		return ret;
	}

	if (!strcmp(pubkey.curve_name, "prime256v1")) {
		nid = NID_X9_62_prime256v1;
	} else if (!strcmp(pubkey.curve_name, "secp384r1")) {
		nid = NID_secp384r1;
	} else {
		fprintf(stderr, "Unsupported curve name: '%s'\n", pubkey.curve_name);
		return -EINVAL;
	}

	fprintf(stderr, "Loading ECDSA key: curve=%s, bits=%d\n", pubkey.curve_name,
		pubkey.size_bits);

	ec_key = EC_KEY_new_by_curve_name(nid);
	if (!ec_key) {
		fprintf(stderr, "Failed to allocate EC_KEY for curve %s\n", pubkey.curve_name);
		return -ENOMEM;
	}

	group = EC_KEY_get0_group(ec_key);
	point = EC_POINT_new(group);
	if (!point) {
		fprintf(stderr, "Failed to allocate EC_POINT\n");
		EC_KEY_free(ec_key);
		return -ENOMEM;
	}

	len = pubkey.size_bits / 8;

	uint8_t buf[1 + len * 2];

	/* uncompressed */
	buf[0] = 0x04;
	memcpy(&buf[1], pubkey.x, len);
	memcpy(&buf[1 + len], pubkey.y, len);
	if (!EC_POINT_oct2point(group, point, buf, sizeof(buf), NULL)) {
		fprintf(stderr, "Failed to convert (x,y) point to EC_POINT\n");
		EC_POINT_free(point);
		EC_KEY_free(ec_key);
		return -EINVAL;
	}

	if (!EC_KEY_set_public_key(ec_key, point)) {
		fprintf(stderr, "Failed to set EC_POINT as public key\n");
		EC_POINT_free(point);
		EC_KEY_free(ec_key);
		return -EINVAL;
	}

	fprintf(stderr, "Successfully loaded ECDSA key from FDT node %d\n", node);
	EC_POINT_free(point);
	ctx->ecdsa_key = ec_key;

	return 0;
}

static int alloc_ctx(struct signer *ctx, const struct image_sign_info *info)
{
	memset(ctx, 0, sizeof(*ctx));

	if (!OPENSSL_init_ssl(0, NULL)) {
		fprintf(stderr, "Failure to init SSL library\n");
		return -1;
	}

	ctx->hash = malloc(info->checksum->checksum_len);
	ctx->signature = malloc(info->crypto->key_len * 2);

	if (!ctx->hash || !ctx->signature)
		return -ENOMEM;

	return 0;
}

static void free_ctx(struct signer *ctx)
{
	if (ctx->ecdsa_key)
		EC_KEY_free(ctx->ecdsa_key);

	if (ctx->evp_key)
		EVP_PKEY_free(ctx->evp_key);

	if (ctx->hash)
		free(ctx->hash);
}

/*
 * Convert an ECDSA signature to raw format
 *
 * openssl DER-encodes 'binary' signatures. We want the signature in a raw
 * (R, S) point pair. So we have to dance a bit.
 */
static void ecdsa_sig_encode_raw(void *buf, const ECDSA_SIG *sig, size_t order)
{
	int point_bytes = order;
	const BIGNUM *r, *s;
	uintptr_t s_buf;

	ECDSA_SIG_get0(sig, &r, &s);
	s_buf = (uintptr_t)buf + point_bytes;
	BN_bn2binpad(r, buf, point_bytes);
	BN_bn2binpad(s, (void *)s_buf, point_bytes);
}

/* Get a signature from a raw encoding */
static ECDSA_SIG *ecdsa_sig_from_raw(void *buf, size_t order)
{
	int point_bytes = order;
	uintptr_t s_buf;
	ECDSA_SIG *sig;
	BIGNUM *r, *s;

	sig = ECDSA_SIG_new();
	if (!sig)
		return NULL;

	s_buf = (uintptr_t)buf + point_bytes;
	r = BN_bin2bn(buf, point_bytes, NULL);
	s = BN_bin2bn((void *)s_buf, point_bytes, NULL);
	ECDSA_SIG_set0(sig, r, s);

	return sig;
}

/* ECDSA key size in bytes */
static size_t ecdsa_key_size_bytes(const EC_KEY *key)
{
	const EC_GROUP *group;

	group = EC_KEY_get0_group(key);
	return (EC_GROUP_order_bits(group) + 7) / 8;
}

static int default_password(char *buf, int size, int rwflag, void *u)
{
	strncpy(buf, (char *)u, size);
	buf[size - 1] = '\0';
	return strlen(buf);
}

static int read_key(struct signer *ctx, const char *key_name)
{
	FILE *f = fopen(key_name, "r");
	const char *key_pass;

	if (!f) {
		fprintf(stderr, "Can not get key file '%s'\n", key_name);
		return -ENOENT;
	}

	key_pass = getenv("MKIMAGE_SIGN_PASSWORD");
	if (key_pass) {
		ctx->evp_key = PEM_read_PrivateKey(f, NULL, default_password, (void *)key_pass);

	} else {
		ctx->evp_key = PEM_read_PrivateKey(f, NULL, NULL, NULL);
	}
	fclose(f);
	if (!ctx->evp_key) {
		fprintf(stderr, "Can not read key from '%s'\n", key_name);
		return -EIO;
	}

	if (EVP_PKEY_id(ctx->evp_key) != EVP_PKEY_EC) {
		fprintf(stderr, "'%s' is not an ECDSA key\n", key_name);
		return -EINVAL;
	}

	ctx->ecdsa_key = EVP_PKEY_get1_EC_KEY(ctx->evp_key);
	if (!ctx->ecdsa_key)
		fprintf(stderr, "Can not extract ECDSA key\n");

	return (ctx->ecdsa_key) ? 0 : -EINVAL;
}

static int load_key_from_fdt(struct signer *ctx, const struct image_sign_info *info)
{
	const void *fdt = info->fdt_blob;
	char name[128];
	int sig_node;
	int key_node;
	int key_len;
	int ret;

	if (!fdt)
		return -EINVAL;

	ret = alloc_ctx(ctx, info);
	if (ret)
		return ret;

	sig_node = fdt_subnode_offset(fdt, 0, FIT_SIG_NODENAME);
	if (sig_node < 0) {
		fprintf(stderr, "No /signature node found\n");
		return -ENOENT;
	}

	/* Case 1: explicitly specified key node */
	if (info->required_keynode >= 0) {
		ret = read_key_from_fdt(ctx, fdt, info->required_keynode);
		if (ret == 0)
			goto check_key_len;

		fprintf(stderr, "Failed to load required keynode %d\n", info->required_keynode);
		return ret;
	}

	/* Case 2: use keyname hint */
	if (info->keyname) {
		snprintf(name, sizeof(name), "%s", info->keyname);
		key_node = fdt_subnode_offset(fdt, sig_node, name);
		if (key_node >= 0) {
			ret = read_key_from_fdt(ctx, fdt, key_node);
			if (ret == 0)
				goto check_key_len;

			fprintf(stderr, "Key hint '%s' found but failed to load\n", info->keyname);
		}
	}

	/* Case 3: try all subnodes */
	fdt_for_each_subnode(key_node, fdt, sig_node) {
		ret = read_key_from_fdt(ctx, fdt, key_node);
		if (ret == 0)
			goto check_key_len;
	}

	fprintf(stderr, "Failed to load any usable ECDSA key from FDT\n");
	return -EINVAL;

check_key_len:
	key_len = ecdsa_key_size_bytes(ctx->ecdsa_key);
	if (key_len != info->crypto->key_len) {
		fprintf(stderr, "Expected %u-bit key, got %u-bit key\n",
			info->crypto->key_len * 8, key_len * 8);
		return -EINVAL;
	}

	return 0;
}

/* Prepare a 'signer' context that's ready to sign and verify. */
static int prepare_ctx(struct signer *ctx, const struct image_sign_info *info)
{
	int key_len_bytes, ret;
	char kname[1024];

	memset(ctx, 0, sizeof(*ctx));

	if (info->fdt_blob) {
		return load_key_from_fdt(ctx, info);
	} else if (info->keyfile) {
		snprintf(kname,  sizeof(kname), "%s", info->keyfile);
	} else if (info->keydir && info->keyname) {
		snprintf(kname, sizeof(kname), "%s/%s.pem", info->keydir,
			 info->keyname);
	} else {
		fprintf(stderr, "keyfile, keyname, or key-name-hint missing\n");
		return -EINVAL;
	}

	ret = alloc_ctx(ctx, info);
	if (ret)
		return ret;

	ret = read_key(ctx, kname);
	if (ret)
		return ret;

	key_len_bytes = ecdsa_key_size_bytes(ctx->ecdsa_key);
	if (key_len_bytes != info->crypto->key_len) {
		fprintf(stderr, "Expected a %u-bit key, got %u-bit key\n",
			info->crypto->key_len * 8, key_len_bytes * 8);
		return -EINVAL;
	}

	return 0;
}

static int do_sign(struct signer *ctx, struct image_sign_info *info,
		   const struct image_region region[], int region_count)
{
	const struct checksum_algo *algo = info->checksum;
	ECDSA_SIG *sig;

	algo->calculate(algo->name, region, region_count, ctx->hash);
	sig = ECDSA_do_sign(ctx->hash, algo->checksum_len, ctx->ecdsa_key);

	ecdsa_sig_encode_raw(ctx->signature, sig, info->crypto->key_len);

	return 0;
}

static int ecdsa_check_signature(struct signer *ctx, struct image_sign_info *info)
{
	ECDSA_SIG *sig;
	int okay;

	sig = ecdsa_sig_from_raw(ctx->signature, info->crypto->key_len);
	if (!sig)
		return -ENOMEM;

	okay = ECDSA_do_verify(ctx->hash, info->checksum->checksum_len,
			       sig, ctx->ecdsa_key);
	if (!okay)
		fprintf(stderr, "WARNING: Signature is fake news!\n");

	ECDSA_SIG_free(sig);
	return !okay;
}

static int do_verify(struct signer *ctx, struct image_sign_info *info,
		     const struct image_region region[], int region_count,
		     uint8_t *raw_sig, uint sig_len)
{
	const struct checksum_algo *algo = info->checksum;

	if (sig_len != info->crypto->key_len * 2) {
		fprintf(stderr, "Signature has wrong length\n");
		return -EINVAL;
	}

	memcpy(ctx->signature, raw_sig, sig_len);
	algo->calculate(algo->name, region, region_count, ctx->hash);

	return ecdsa_check_signature(ctx, info);
}

int ecdsa_sign(struct image_sign_info *info, const struct image_region region[],
	       int region_count, uint8_t **sigp, uint *sig_len)
{
	struct signer ctx;
	int ret;

	ret = prepare_ctx(&ctx, info);
	if (ret >= 0) {
		do_sign(&ctx, info, region, region_count);
		*sigp = ctx.signature;
		*sig_len = info->crypto->key_len * 2;

		ret = ecdsa_check_signature(&ctx, info);
	}

	free_ctx(&ctx);
	return ret;
}

int ecdsa_verify(struct image_sign_info *info,
		 const struct image_region region[], int region_count,
		 uint8_t *sig, uint sig_len)
{
	struct signer ctx;
	int ret;

	ret = prepare_ctx(&ctx, info);
	if (ret >= 0)
		ret = do_verify(&ctx, info, region, region_count, sig, sig_len);

	free_ctx(&ctx);
	return ret;
}

static int do_add(struct signer *ctx, void *fdt, const char *key_node_name,
		  struct image_sign_info *info)
{
	int signature_node, key_node, ret, key_bits;
	const char *curve_name;
	const EC_GROUP *group;
	const EC_POINT *point;
	BIGNUM *x, *y;

	signature_node = fdt_subnode_offset(fdt, 0, FIT_SIG_NODENAME);
	if (signature_node == -FDT_ERR_NOTFOUND) {
		signature_node = fdt_add_subnode(fdt, 0, FIT_SIG_NODENAME);
		if (signature_node < 0) {
			if (signature_node != -FDT_ERR_NOSPACE) {
				fprintf(stderr, "Couldn't create signature node: %s\n",
					fdt_strerror(signature_node));
			}
			return signature_node;
		}
	} else if (signature_node < 0) {
		fprintf(stderr, "Cannot select keys signature_node: %s\n",
			fdt_strerror(signature_node));
		return signature_node;
	}

	/* Either create or overwrite the named key node */
	key_node = fdt_subnode_offset(fdt, signature_node, key_node_name);
	if (key_node == -FDT_ERR_NOTFOUND) {
		key_node = fdt_add_subnode(fdt, signature_node, key_node_name);
		if (key_node < 0) {
			if (key_node != -FDT_ERR_NOSPACE) {
				fprintf(stderr, "Could not create key subnode: %s\n",
					fdt_strerror(key_node));
			}
			return key_node;
		}
	} else if (key_node < 0) {
		fprintf(stderr, "Cannot select keys key_node: %s\n",
			fdt_strerror(key_node));
		return key_node;
	}

	group = EC_KEY_get0_group(ctx->ecdsa_key);
	key_bits = EC_GROUP_order_bits(group);
	curve_name = OBJ_nid2sn(EC_GROUP_get_curve_name(group));
	/* Let 'x' and 'y' memory leak by not BN_free()'ing them. */
	x = BN_new();
	y = BN_new();
	point = EC_KEY_get0_public_key(ctx->ecdsa_key);
	EC_POINT_get_affine_coordinates(group, point, x, y, NULL);

	ret = fdt_setprop_string(fdt, key_node, FIT_KEY_HINT,
				 info->keyname);
	if (ret < 0)
		return ret;

	ret = fdt_setprop_string(fdt, key_node, "ecdsa,curve", curve_name);
	if (ret < 0)
		return ret;

	ret = fdt_add_bignum(fdt, key_node, "ecdsa,x-point", x, key_bits);
	if (ret < 0)
		return ret;

	ret = fdt_add_bignum(fdt, key_node, "ecdsa,y-point", y, key_bits);
	if (ret < 0)
		return ret;

	ret = fdt_setprop_string(fdt, key_node, FIT_ALGO_PROP,
				 info->name);
	if (ret < 0)
		return ret;

	if (info->require_keys) {
		ret = fdt_setprop_string(fdt, key_node, FIT_KEY_REQUIRED,
					 info->require_keys);
		if (ret < 0)
			return ret;
	}

	return key_node;
}

int ecdsa_add_verify_data(struct image_sign_info *info, void *fdt)
{
	const char *fdt_key_name;
	struct signer ctx;
	int ret;

	fdt_key_name = info->keyname ? info->keyname : "default-key";
	ret = prepare_ctx(&ctx, info);
	if (ret >= 0) {
		ret = do_add(&ctx, fdt, fdt_key_name, info);
		if (ret < 0) {
			free_ctx(&ctx);
			return ret == -FDT_ERR_NOSPACE ? -ENOSPC : -EIO;
		}
	}

	free_ctx(&ctx);
	return ret;
}
