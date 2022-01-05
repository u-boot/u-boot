// SPDX-License-Identifier: GPL-2.0+
/*
 * Image manipulator for Marvell SoCs
 *  supports Kirkwood, Dove, Armada 370, Armada XP, Armada 375, Armada 38x and
 *  Armada 39x
 *
 * (C) Copyright 2013 Thomas Petazzoni
 * <thomas.petazzoni@free-electrons.com>
 */

#define OPENSSL_API_COMPAT 0x10101000L

#include "imagetool.h"
#include <limits.h>
#include <image.h>
#include <stdarg.h>
#include <stdint.h>
#include "kwbimage.h"

#include <openssl/bn.h>
#include <openssl/rsa.h>
#include <openssl/pem.h>
#include <openssl/err.h>
#include <openssl/evp.h>

#if OPENSSL_VERSION_NUMBER < 0x10100000L || \
    (defined(LIBRESSL_VERSION_NUMBER) && LIBRESSL_VERSION_NUMBER < 0x2070000fL)
static void RSA_get0_key(const RSA *r,
                 const BIGNUM **n, const BIGNUM **e, const BIGNUM **d)
{
   if (n != NULL)
       *n = r->n;
   if (e != NULL)
       *e = r->e;
   if (d != NULL)
       *d = r->d;
}

#elif !defined(LIBRESSL_VERSION_NUMBER)
void EVP_MD_CTX_cleanup(EVP_MD_CTX *ctx)
{
	EVP_MD_CTX_reset(ctx);
}
#endif

static struct image_cfg_element *image_cfg;
static int cfgn;
static int verbose_mode;

struct boot_mode {
	unsigned int id;
	const char *name;
};

/*
 * SHA2-256 hash
 */
struct hash_v1 {
	uint8_t hash[32];
};

struct boot_mode boot_modes[] = {
	{ IBR_HDR_I2C_ID, "i2c"  },
	{ IBR_HDR_SPI_ID, "spi"  },
	{ IBR_HDR_NAND_ID, "nand" },
	{ IBR_HDR_SATA_ID, "sata" },
	{ IBR_HDR_PEX_ID, "pex"  },
	{ IBR_HDR_UART_ID, "uart" },
	{ IBR_HDR_SDIO_ID, "sdio" },
	{},
};

struct nand_ecc_mode {
	unsigned int id;
	const char *name;
};

struct nand_ecc_mode nand_ecc_modes[] = {
	{ IBR_HDR_ECC_DEFAULT, "default" },
	{ IBR_HDR_ECC_FORCED_HAMMING, "hamming" },
	{ IBR_HDR_ECC_FORCED_RS, "rs" },
	{ IBR_HDR_ECC_DISABLED, "disabled" },
	{},
};

/* Used to identify an undefined execution or destination address */
#define ADDR_INVALID ((uint32_t)-1)

#define BINARY_MAX_ARGS 255

/* In-memory representation of a line of the configuration file */

enum image_cfg_type {
	IMAGE_CFG_VERSION = 0x1,
	IMAGE_CFG_BOOT_FROM,
	IMAGE_CFG_DEST_ADDR,
	IMAGE_CFG_EXEC_ADDR,
	IMAGE_CFG_NAND_BLKSZ,
	IMAGE_CFG_NAND_BADBLK_LOCATION,
	IMAGE_CFG_NAND_ECC_MODE,
	IMAGE_CFG_NAND_PAGESZ,
	IMAGE_CFG_BINARY,
	IMAGE_CFG_DATA,
	IMAGE_CFG_DATA_DELAY,
	IMAGE_CFG_BAUDRATE,
	IMAGE_CFG_UART_PORT,
	IMAGE_CFG_UART_MPP,
	IMAGE_CFG_DEBUG,
	IMAGE_CFG_KAK,
	IMAGE_CFG_CSK,
	IMAGE_CFG_CSK_INDEX,
	IMAGE_CFG_JTAG_DELAY,
	IMAGE_CFG_BOX_ID,
	IMAGE_CFG_FLASH_ID,
	IMAGE_CFG_SEC_COMMON_IMG,
	IMAGE_CFG_SEC_SPECIALIZED_IMG,
	IMAGE_CFG_SEC_BOOT_DEV,
	IMAGE_CFG_SEC_FUSE_DUMP,

	IMAGE_CFG_COUNT
} type;

static const char * const id_strs[] = {
	[IMAGE_CFG_VERSION] = "VERSION",
	[IMAGE_CFG_BOOT_FROM] = "BOOT_FROM",
	[IMAGE_CFG_DEST_ADDR] = "DEST_ADDR",
	[IMAGE_CFG_EXEC_ADDR] = "EXEC_ADDR",
	[IMAGE_CFG_NAND_BLKSZ] = "NAND_BLKSZ",
	[IMAGE_CFG_NAND_BADBLK_LOCATION] = "NAND_BADBLK_LOCATION",
	[IMAGE_CFG_NAND_ECC_MODE] = "NAND_ECC_MODE",
	[IMAGE_CFG_NAND_PAGESZ] = "NAND_PAGE_SIZE",
	[IMAGE_CFG_BINARY] = "BINARY",
	[IMAGE_CFG_DATA] = "DATA",
	[IMAGE_CFG_DATA_DELAY] = "DATA_DELAY",
	[IMAGE_CFG_BAUDRATE] = "BAUDRATE",
	[IMAGE_CFG_UART_PORT] = "UART_PORT",
	[IMAGE_CFG_UART_MPP] = "UART_MPP",
	[IMAGE_CFG_DEBUG] = "DEBUG",
	[IMAGE_CFG_KAK] = "KAK",
	[IMAGE_CFG_CSK] = "CSK",
	[IMAGE_CFG_CSK_INDEX] = "CSK_INDEX",
	[IMAGE_CFG_JTAG_DELAY] = "JTAG_DELAY",
	[IMAGE_CFG_BOX_ID] = "BOX_ID",
	[IMAGE_CFG_FLASH_ID] = "FLASH_ID",
	[IMAGE_CFG_SEC_COMMON_IMG] = "SEC_COMMON_IMG",
	[IMAGE_CFG_SEC_SPECIALIZED_IMG] = "SEC_SPECIALIZED_IMG",
	[IMAGE_CFG_SEC_BOOT_DEV] = "SEC_BOOT_DEV",
	[IMAGE_CFG_SEC_FUSE_DUMP] = "SEC_FUSE_DUMP"
};

struct image_cfg_element {
	enum image_cfg_type type;
	union {
		unsigned int version;
		unsigned int bootfrom;
		struct {
			const char *file;
			unsigned int args[BINARY_MAX_ARGS];
			unsigned int nargs;
		} binary;
		unsigned int dstaddr;
		unsigned int execaddr;
		unsigned int nandblksz;
		unsigned int nandbadblklocation;
		unsigned int nandeccmode;
		unsigned int nandpagesz;
		struct ext_hdr_v0_reg regdata;
		unsigned int regdata_delay;
		unsigned int baudrate;
		unsigned int uart_port;
		unsigned int uart_mpp;
		unsigned int debug;
		const char *key_name;
		int csk_idx;
		uint8_t jtag_delay;
		uint32_t boxid;
		uint32_t flashid;
		bool sec_specialized_img;
		unsigned int sec_boot_dev;
		const char *name;
	};
};

#define IMAGE_CFG_ELEMENT_MAX 256

/*
 * Utility functions to manipulate boot mode and ecc modes (convert
 * them back and forth between description strings and the
 * corresponding numerical identifiers).
 */

static const char *image_boot_mode_name(unsigned int id)
{
	int i;

	for (i = 0; boot_modes[i].name; i++)
		if (boot_modes[i].id == id)
			return boot_modes[i].name;
	return NULL;
}

int image_boot_mode_id(const char *boot_mode_name)
{
	int i;

	for (i = 0; boot_modes[i].name; i++)
		if (!strcmp(boot_modes[i].name, boot_mode_name))
			return boot_modes[i].id;

	return -1;
}

int image_nand_ecc_mode_id(const char *nand_ecc_mode_name)
{
	int i;

	for (i = 0; nand_ecc_modes[i].name; i++)
		if (!strcmp(nand_ecc_modes[i].name, nand_ecc_mode_name))
			return nand_ecc_modes[i].id;
	return -1;
}

static struct image_cfg_element *
image_find_option(unsigned int optiontype)
{
	int i;

	for (i = 0; i < cfgn; i++) {
		if (image_cfg[i].type == optiontype)
			return &image_cfg[i];
	}

	return NULL;
}

static unsigned int
image_count_options(unsigned int optiontype)
{
	int i;
	unsigned int count = 0;

	for (i = 0; i < cfgn; i++)
		if (image_cfg[i].type == optiontype)
			count++;

	return count;
}

static int image_get_csk_index(void)
{
	struct image_cfg_element *e;

	e = image_find_option(IMAGE_CFG_CSK_INDEX);
	if (!e)
		return -1;

	return e->csk_idx;
}

static bool image_get_spezialized_img(void)
{
	struct image_cfg_element *e;

	e = image_find_option(IMAGE_CFG_SEC_SPECIALIZED_IMG);
	if (!e)
		return false;

	return e->sec_specialized_img;
}

static int image_get_bootfrom(void)
{
	struct image_cfg_element *e;

	e = image_find_option(IMAGE_CFG_BOOT_FROM);
	if (!e)
		/* fallback to SPI if no BOOT_FROM is not provided */
		return IBR_HDR_SPI_ID;

	return e->bootfrom;
}

/*
 * Compute a 8-bit checksum of a memory area. This algorithm follows
 * the requirements of the Marvell SoC BootROM specifications.
 */
static uint8_t image_checksum8(void *start, uint32_t len)
{
	uint8_t csum = 0;
	uint8_t *p = start;

	/* check len and return zero checksum if invalid */
	if (!len)
		return 0;

	do {
		csum += *p;
		p++;
	} while (--len);

	return csum;
}

/*
 * Verify checksum over a complete header that includes the checksum field.
 * Return 1 when OK, otherwise 0.
 */
static int main_hdr_checksum_ok(void *hdr)
{
	/* Offsets of checksum in v0 and v1 headers are the same */
	struct main_hdr_v0 *main_hdr = (struct main_hdr_v0 *)hdr;
	uint8_t checksum;

	checksum = image_checksum8(hdr, kwbheader_size_for_csum(hdr));
	/* Calculated checksum includes the header checksum field. Compensate
	 * for that.
	 */
	checksum -= main_hdr->checksum;

	return checksum == main_hdr->checksum;
}

static uint32_t image_checksum32(void *start, uint32_t len)
{
	uint32_t csum = 0;
	uint32_t *p = start;

	/* check len and return zero checksum if invalid */
	if (!len)
		return 0;

	if (len % sizeof(uint32_t)) {
		fprintf(stderr, "Length %d is not in multiple of %zu\n",
			len, sizeof(uint32_t));
		return 0;
	}

	do {
		csum += *p;
		p++;
		len -= sizeof(uint32_t);
	} while (len > 0);

	return csum;
}

static uint8_t baudrate_to_option(unsigned int baudrate)
{
	switch (baudrate) {
	case 2400:
		return MAIN_HDR_V1_OPT_BAUD_2400;
	case 4800:
		return MAIN_HDR_V1_OPT_BAUD_4800;
	case 9600:
		return MAIN_HDR_V1_OPT_BAUD_9600;
	case 19200:
		return MAIN_HDR_V1_OPT_BAUD_19200;
	case 38400:
		return MAIN_HDR_V1_OPT_BAUD_38400;
	case 57600:
		return MAIN_HDR_V1_OPT_BAUD_57600;
	case 115200:
		return MAIN_HDR_V1_OPT_BAUD_115200;
	default:
		return MAIN_HDR_V1_OPT_BAUD_DEFAULT;
	}
}

static void kwb_msg(const char *fmt, ...)
{
	if (verbose_mode) {
		va_list ap;

		va_start(ap, fmt);
		vfprintf(stdout, fmt, ap);
		va_end(ap);
	}
}

static int openssl_err(const char *msg)
{
	unsigned long ssl_err = ERR_get_error();

	fprintf(stderr, "%s", msg);
	fprintf(stderr, ": %s\n",
		ERR_error_string(ssl_err, 0));

	return -1;
}

static int kwb_load_rsa_key(const char *keydir, const char *name, RSA **p_rsa)
{
	char path[PATH_MAX];
	RSA *rsa;
	FILE *f;

	if (!keydir)
		keydir = ".";

	snprintf(path, sizeof(path), "%s/%s.key", keydir, name);
	f = fopen(path, "r");
	if (!f) {
		fprintf(stderr, "Couldn't open RSA private key: '%s': %s\n",
			path, strerror(errno));
		return -ENOENT;
	}

	rsa = PEM_read_RSAPrivateKey(f, 0, NULL, "");
	if (!rsa) {
		openssl_err("Failure reading private key");
		fclose(f);
		return -EPROTO;
	}
	fclose(f);
	*p_rsa = rsa;

	return 0;
}

static int kwb_load_cfg_key(struct image_tool_params *params,
			    unsigned int cfg_option, const char *key_name,
			    RSA **p_key)
{
	struct image_cfg_element *e_key;
	RSA *key;
	int res;

	*p_key = NULL;

	e_key = image_find_option(cfg_option);
	if (!e_key) {
		fprintf(stderr, "%s not configured\n", key_name);
		return -ENOENT;
	}

	res = kwb_load_rsa_key(params->keydir, e_key->key_name, &key);
	if (res < 0) {
		fprintf(stderr, "Failed to load %s\n", key_name);
		return -ENOENT;
	}

	*p_key = key;

	return 0;
}

static int kwb_load_kak(struct image_tool_params *params, RSA **p_kak)
{
	return kwb_load_cfg_key(params, IMAGE_CFG_KAK, "KAK", p_kak);
}

static int kwb_load_csk(struct image_tool_params *params, RSA **p_csk)
{
	return kwb_load_cfg_key(params, IMAGE_CFG_CSK, "CSK", p_csk);
}

static int kwb_compute_pubkey_hash(struct pubkey_der_v1 *pk,
				   struct hash_v1 *hash)
{
	EVP_MD_CTX *ctx;
	unsigned int key_size;
	unsigned int hash_size;
	int ret = 0;

	if (!pk || !hash || pk->key[0] != 0x30 || pk->key[1] != 0x82)
		return -EINVAL;

	key_size = (pk->key[2] << 8) + pk->key[3] + 4;

	ctx = EVP_MD_CTX_create();
	if (!ctx)
		return openssl_err("EVP context creation failed");

	EVP_MD_CTX_init(ctx);
	if (!EVP_DigestInit(ctx, EVP_sha256())) {
		ret = openssl_err("Digest setup failed");
		goto hash_err_ctx;
	}

	if (!EVP_DigestUpdate(ctx, pk->key, key_size)) {
		ret = openssl_err("Hashing data failed");
		goto hash_err_ctx;
	}

	if (!EVP_DigestFinal(ctx, hash->hash, &hash_size)) {
		ret = openssl_err("Could not obtain hash");
		goto hash_err_ctx;
	}

	EVP_MD_CTX_cleanup(ctx);

hash_err_ctx:
	EVP_MD_CTX_destroy(ctx);
	return ret;
}

static int kwb_import_pubkey(RSA **key, struct pubkey_der_v1 *src, char *keyname)
{
	RSA *rsa;
	const unsigned char *ptr;

	if (!key || !src)
		goto fail;

	ptr = src->key;
	rsa = d2i_RSAPublicKey(key, &ptr, sizeof(src->key));
	if (!rsa) {
		openssl_err("error decoding public key");
		goto fail;
	}

	return 0;
fail:
	fprintf(stderr, "Failed to decode %s pubkey\n", keyname);
	return -EINVAL;
}

static int kwb_export_pubkey(RSA *key, struct pubkey_der_v1 *dst, FILE *hashf,
			     char *keyname)
{
	int size_exp, size_mod, size_seq;
	const BIGNUM *key_e, *key_n;
	uint8_t *cur;
	char *errmsg = "Failed to encode %s\n";

	RSA_get0_key(key, NULL, &key_e, NULL);
	RSA_get0_key(key, &key_n, NULL, NULL);

	if (!key || !key_e || !key_n || !dst) {
		fprintf(stderr, "export pk failed: (%p, %p, %p, %p)",
			key, key_e, key_n, dst);
		fprintf(stderr, errmsg, keyname);
		return -EINVAL;
	}

	/*
	 * According to the specs, the key should be PKCS#1 DER encoded.
	 * But unfortunately the really required encoding seems to be different;
	 * it violates DER...! (But it still conformes to BER.)
	 * (Length always in long form w/ 2 byte length code; no leading zero
	 * when MSB of first byte is set...)
	 * So we cannot use the encoding func provided by OpenSSL and have to
	 * do the encoding manually.
	 */

	size_exp = BN_num_bytes(key_e);
	size_mod = BN_num_bytes(key_n);
	size_seq = 4 + size_mod + 4 + size_exp;

	if (size_mod > 256) {
		fprintf(stderr, "export pk failed: wrong mod size: %d\n",
			size_mod);
		fprintf(stderr, errmsg, keyname);
		return -EINVAL;
	}

	if (4 + size_seq > sizeof(dst->key)) {
		fprintf(stderr, "export pk failed: seq too large (%d, %zu)\n",
			4 + size_seq, sizeof(dst->key));
		fprintf(stderr, errmsg, keyname);
		return -ENOBUFS;
	}

	cur = dst->key;

	/* PKCS#1 (RFC3447) RSAPublicKey structure */
	*cur++ = 0x30;		/* SEQUENCE */
	*cur++ = 0x82;
	*cur++ = (size_seq >> 8) & 0xFF;
	*cur++ = size_seq & 0xFF;
	/* Modulus */
	*cur++ = 0x02;		/* INTEGER */
	*cur++ = 0x82;
	*cur++ = (size_mod >> 8) & 0xFF;
	*cur++ = size_mod & 0xFF;
	BN_bn2bin(key_n, cur);
	cur += size_mod;
	/* Exponent */
	*cur++ = 0x02;		/* INTEGER */
	*cur++ = 0x82;
	*cur++ = (size_exp >> 8) & 0xFF;
	*cur++ = size_exp & 0xFF;
	BN_bn2bin(key_e, cur);

	if (hashf) {
		struct hash_v1 pk_hash;
		int i;
		int ret = 0;

		ret = kwb_compute_pubkey_hash(dst, &pk_hash);
		if (ret < 0) {
			fprintf(stderr, errmsg, keyname);
			return ret;
		}

		fprintf(hashf, "SHA256 = ");
		for (i = 0 ; i < sizeof(pk_hash.hash); ++i)
			fprintf(hashf, "%02X", pk_hash.hash[i]);
		fprintf(hashf, "\n");
	}

	return 0;
}

int kwb_sign(RSA *key, void *data, int datasz, struct sig_v1 *sig, char *signame)
{
	EVP_PKEY *evp_key;
	EVP_MD_CTX *ctx;
	unsigned int sig_size;
	int size;
	int ret = 0;

	evp_key = EVP_PKEY_new();
	if (!evp_key)
		return openssl_err("EVP_PKEY object creation failed");

	if (!EVP_PKEY_set1_RSA(evp_key, key)) {
		ret = openssl_err("EVP key setup failed");
		goto err_key;
	}

	size = EVP_PKEY_size(evp_key);
	if (size > sizeof(sig->sig)) {
		fprintf(stderr, "Buffer to small for signature (%d bytes)\n",
			size);
		ret = -ENOBUFS;
		goto err_key;
	}

	ctx = EVP_MD_CTX_create();
	if (!ctx) {
		ret = openssl_err("EVP context creation failed");
		goto err_key;
	}
	EVP_MD_CTX_init(ctx);
	if (!EVP_SignInit(ctx, EVP_sha256())) {
		ret = openssl_err("Signer setup failed");
		goto err_ctx;
	}

	if (!EVP_SignUpdate(ctx, data, datasz)) {
		ret = openssl_err("Signing data failed");
		goto err_ctx;
	}

	if (!EVP_SignFinal(ctx, sig->sig, &sig_size, evp_key)) {
		ret = openssl_err("Could not obtain signature");
		goto err_ctx;
	}

	EVP_MD_CTX_cleanup(ctx);
	EVP_MD_CTX_destroy(ctx);
	EVP_PKEY_free(evp_key);

	return 0;

err_ctx:
	EVP_MD_CTX_destroy(ctx);
err_key:
	EVP_PKEY_free(evp_key);
	fprintf(stderr, "Failed to create %s signature\n", signame);
	return ret;
}

int kwb_verify(RSA *key, void *data, int datasz, struct sig_v1 *sig,
	       char *signame)
{
	EVP_PKEY *evp_key;
	EVP_MD_CTX *ctx;
	int size;
	int ret = 0;

	evp_key = EVP_PKEY_new();
	if (!evp_key)
		return openssl_err("EVP_PKEY object creation failed");

	if (!EVP_PKEY_set1_RSA(evp_key, key)) {
		ret = openssl_err("EVP key setup failed");
		goto err_key;
	}

	size = EVP_PKEY_size(evp_key);
	if (size > sizeof(sig->sig)) {
		fprintf(stderr, "Invalid signature size (%d bytes)\n",
			size);
		ret = -EINVAL;
		goto err_key;
	}

	ctx = EVP_MD_CTX_create();
	if (!ctx) {
		ret = openssl_err("EVP context creation failed");
		goto err_key;
	}
	EVP_MD_CTX_init(ctx);
	if (!EVP_VerifyInit(ctx, EVP_sha256())) {
		ret = openssl_err("Verifier setup failed");
		goto err_ctx;
	}

	if (!EVP_VerifyUpdate(ctx, data, datasz)) {
		ret = openssl_err("Hashing data failed");
		goto err_ctx;
	}

	if (EVP_VerifyFinal(ctx, sig->sig, sizeof(sig->sig), evp_key) != 1) {
		ret = openssl_err("Could not verify signature");
		goto err_ctx;
	}

	EVP_MD_CTX_cleanup(ctx);
	EVP_MD_CTX_destroy(ctx);
	EVP_PKEY_free(evp_key);

	return 0;

err_ctx:
	EVP_MD_CTX_destroy(ctx);
err_key:
	EVP_PKEY_free(evp_key);
	fprintf(stderr, "Failed to verify %s signature\n", signame);
	return ret;
}

int kwb_sign_and_verify(RSA *key, void *data, int datasz, struct sig_v1 *sig,
			char *signame)
{
	if (kwb_sign(key, data, datasz, sig, signame) < 0)
		return -1;

	if (kwb_verify(key, data, datasz, sig, signame) < 0)
		return -1;

	return 0;
}


int kwb_dump_fuse_cmds_38x(FILE *out, struct secure_hdr_v1 *sec_hdr)
{
	struct hash_v1 kak_pub_hash;
	struct image_cfg_element *e;
	unsigned int fuse_line;
	int i, idx;
	uint8_t *ptr;
	uint32_t val;
	int ret = 0;

	if (!out || !sec_hdr)
		return -EINVAL;

	ret = kwb_compute_pubkey_hash(&sec_hdr->kak, &kak_pub_hash);
	if (ret < 0)
		goto done;

	fprintf(out, "# burn KAK pub key hash\n");
	ptr = kak_pub_hash.hash;
	for (fuse_line = 26; fuse_line <= 30; ++fuse_line) {
		fprintf(out, "fuse prog -y %u 0 ", fuse_line);

		for (i = 4; i-- > 0;)
			fprintf(out, "%02hx", (ushort)ptr[i]);
		ptr += 4;
		fprintf(out, " 00");

		if (fuse_line < 30) {
			for (i = 3; i-- > 0;)
				fprintf(out, "%02hx", (ushort)ptr[i]);
			ptr += 3;
		} else {
			fprintf(out, "000000");
		}

		fprintf(out, " 1\n");
	}

	fprintf(out, "# burn CSK selection\n");

	idx = image_get_csk_index();
	if (idx < 0 || idx > 15) {
		ret = -EINVAL;
		goto done;
	}
	if (idx > 0) {
		for (fuse_line = 31; fuse_line < 31 + idx; ++fuse_line)
			fprintf(out, "fuse prog -y %u 0 00000001 00000000 1\n",
				fuse_line);
	} else {
		fprintf(out, "# CSK index is 0; no mods needed\n");
	}

	e = image_find_option(IMAGE_CFG_BOX_ID);
	if (e) {
		fprintf(out, "# set box ID\n");
		fprintf(out, "fuse prog -y 48 0 %08x 00000000 1\n", e->boxid);
	}

	e = image_find_option(IMAGE_CFG_FLASH_ID);
	if (e) {
		fprintf(out, "# set flash ID\n");
		fprintf(out, "fuse prog -y 47 0 %08x 00000000 1\n", e->flashid);
	}

	fprintf(out, "# enable secure mode ");
	fprintf(out, "(must be the last fuse line written)\n");

	val = 1;
	e = image_find_option(IMAGE_CFG_SEC_BOOT_DEV);
	if (!e) {
		fprintf(stderr, "ERROR: secured mode boot device not given\n");
		ret = -EINVAL;
		goto done;
	}

	if (e->sec_boot_dev > 0xff) {
		fprintf(stderr, "ERROR: secured mode boot device invalid\n");
		ret = -EINVAL;
		goto done;
	}

	val |= (e->sec_boot_dev << 8);

	fprintf(out, "fuse prog -y 24 0 %08x 0103e0a9 1\n", val);

	fprintf(out, "# lock (unused) fuse lines (0-23)s\n");
	for (fuse_line = 0; fuse_line < 24; ++fuse_line)
		fprintf(out, "fuse prog -y %u 2 1\n", fuse_line);

	fprintf(out, "# OK, that's all :-)\n");

done:
	return ret;
}

static int kwb_dump_fuse_cmds(struct secure_hdr_v1 *sec_hdr)
{
	int ret = 0;
	struct image_cfg_element *e;

	e = image_find_option(IMAGE_CFG_SEC_FUSE_DUMP);
	if (!e)
		return 0;

	if (!strcmp(e->name, "a38x")) {
		FILE *out = fopen("kwb_fuses_a38x.txt", "w+");

		if (!out) {
			fprintf(stderr, "Couldn't open eFuse settings: '%s': %s\n",
				"kwb_fuses_a38x.txt", strerror(errno));
			return -ENOENT;
		}

		kwb_dump_fuse_cmds_38x(out, sec_hdr);
		fclose(out);
		goto done;
	}

	ret = -ENOSYS;

done:
	return ret;
}

static size_t image_headersz_align(size_t headersz, uint8_t blockid)
{
	/*
	 * Header needs to be 4-byte aligned, which is already ensured by code
	 * above. Moreover UART images must have header aligned to 128 bytes
	 * (xmodem block size), NAND images to 256 bytes (ECC calculation),
	 * and SATA and SDIO images to 512 bytes (storage block size).
	 * Note that SPI images do not have to have header size aligned
	 * to 256 bytes because it is possible to read from SPI storage from
	 * any offset (read offset does not have to be aligned to block size).
	 */
	if (blockid == IBR_HDR_UART_ID)
		return ALIGN(headersz, 128);
	else if (blockid == IBR_HDR_NAND_ID)
		return ALIGN(headersz, 256);
	else if (blockid == IBR_HDR_SATA_ID || blockid == IBR_HDR_SDIO_ID)
		return ALIGN(headersz, 512);
	else
		return headersz;
}

static size_t image_headersz_v0(int *hasext)
{
	size_t headersz;

	headersz = sizeof(struct main_hdr_v0);
	if (image_count_options(IMAGE_CFG_DATA) > 0) {
		headersz += sizeof(struct ext_hdr_v0);
		if (hasext)
			*hasext = 1;
	}

	return image_headersz_align(headersz, image_get_bootfrom());
}

static void *image_create_v0(size_t *imagesz, struct image_tool_params *params,
			     int payloadsz)
{
	struct image_cfg_element *e;
	size_t headersz;
	struct main_hdr_v0 *main_hdr;
	uint8_t *image;
	int has_ext = 0;

	/*
	 * Calculate the size of the header and the size of the
	 * payload
	 */
	headersz = image_headersz_v0(&has_ext);

	image = malloc(headersz);
	if (!image) {
		fprintf(stderr, "Cannot allocate memory for image\n");
		return NULL;
	}

	memset(image, 0, headersz);

	main_hdr = (struct main_hdr_v0 *)image;

	/* Fill in the main header */
	main_hdr->blocksize =
		cpu_to_le32(payloadsz);
	main_hdr->srcaddr   = cpu_to_le32(headersz);
	main_hdr->ext       = has_ext;
	main_hdr->version   = 0;
	main_hdr->destaddr  = cpu_to_le32(params->addr);
	main_hdr->execaddr  = cpu_to_le32(params->ep);
	main_hdr->blockid   = image_get_bootfrom();

	e = image_find_option(IMAGE_CFG_NAND_ECC_MODE);
	if (e)
		main_hdr->nandeccmode = e->nandeccmode;
	e = image_find_option(IMAGE_CFG_NAND_PAGESZ);
	if (e)
		main_hdr->nandpagesize = cpu_to_le16(e->nandpagesz);
	main_hdr->checksum = image_checksum8(image,
					     sizeof(struct main_hdr_v0));

	/*
	 * For SATA srcaddr is specified in number of sectors starting from
	 * sector 0. The main header is stored at sector number 1.
	 * This expects the sector size to be 512 bytes.
	 * Header size is already aligned.
	 */
	if (main_hdr->blockid == IBR_HDR_SATA_ID)
		main_hdr->srcaddr = cpu_to_le32(headersz / 512 + 1);

	/*
	 * For SDIO srcaddr is specified in number of sectors starting from
	 * sector 0. The main header is stored at sector number 0.
	 * This expects sector size to be 512 bytes.
	 * Header size is already aligned.
	 */
	if (main_hdr->blockid == IBR_HDR_SDIO_ID)
		main_hdr->srcaddr = cpu_to_le32(headersz / 512);

	/* For PCIe srcaddr is not used and must be set to 0xFFFFFFFF. */
	if (main_hdr->blockid == IBR_HDR_PEX_ID)
		main_hdr->srcaddr = cpu_to_le32(0xFFFFFFFF);

	/* Generate the ext header */
	if (has_ext) {
		struct ext_hdr_v0 *ext_hdr;
		int cfgi, datai;

		ext_hdr = (struct ext_hdr_v0 *)
				(image + sizeof(struct main_hdr_v0));
		ext_hdr->offset = cpu_to_le32(0x40);

		for (cfgi = 0, datai = 0; cfgi < cfgn; cfgi++) {
			e = &image_cfg[cfgi];
			if (e->type != IMAGE_CFG_DATA)
				continue;

			ext_hdr->rcfg[datai].raddr =
				cpu_to_le32(e->regdata.raddr);
			ext_hdr->rcfg[datai].rdata =
				cpu_to_le32(e->regdata.rdata);
			datai++;
		}

		ext_hdr->checksum = image_checksum8(ext_hdr,
						    sizeof(struct ext_hdr_v0));
	}

	*imagesz = headersz;
	return image;
}

static size_t image_headersz_v1(int *hasext)
{
	struct image_cfg_element *binarye;
	unsigned int count;
	size_t headersz;
	int cfgi;

	/*
	 * Calculate the size of the header and the size of the
	 * payload
	 */
	headersz = sizeof(struct main_hdr_v1);

	if (image_get_csk_index() >= 0) {
		headersz += sizeof(struct secure_hdr_v1);
		if (hasext)
			*hasext = 1;
	}

	count = image_count_options(IMAGE_CFG_DATA);
	if (count > 0)
		headersz += sizeof(struct register_set_hdr_v1) + 8 * count + 4;

	for (cfgi = 0; cfgi < cfgn; cfgi++) {
		int ret;
		struct stat s;

		binarye = &image_cfg[cfgi];
		if (binarye->type != IMAGE_CFG_BINARY)
			continue;

		ret = stat(binarye->binary.file, &s);
		if (ret < 0) {
			char cwd[PATH_MAX];
			char *dir = cwd;

			memset(cwd, 0, sizeof(cwd));
			if (!getcwd(cwd, sizeof(cwd))) {
				dir = "current working directory";
				perror("getcwd() failed");
			}

			fprintf(stderr,
				"Didn't find the file '%s' in '%s' which is mandatory to generate the image\n"
				"This file generally contains the DDR3 training code, and should be extracted from an existing bootable\n"
				"image for your board. Use 'dumpimage -T kwbimage -p 0' to extract it from an existing image.\n",
				binarye->binary.file, dir);
			return 0;
		}

		headersz += sizeof(struct opt_hdr_v1) + sizeof(uint32_t) +
			(binarye->binary.nargs) * sizeof(uint32_t);
		headersz = ALIGN(headersz, 16);
		headersz += ALIGN(s.st_size, 4) + sizeof(uint32_t);
		if (hasext)
			*hasext = 1;
	}

	return image_headersz_align(headersz, image_get_bootfrom());
}

int add_binary_header_v1(uint8_t **cur, uint8_t **next_ext,
			 struct image_cfg_element *binarye,
			 struct main_hdr_v1 *main_hdr)
{
	struct opt_hdr_v1 *hdr = (struct opt_hdr_v1 *)*cur;
	uint32_t add_args;
	uint32_t offset;
	uint32_t *args;
	size_t binhdrsz;
	struct stat s;
	int argi;
	FILE *bin;
	int ret;

	hdr->headertype = OPT_HDR_V1_BINARY_TYPE;

	bin = fopen(binarye->binary.file, "r");
	if (!bin) {
		fprintf(stderr, "Cannot open binary file %s\n",
			binarye->binary.file);
		return -1;
	}

	if (fstat(fileno(bin), &s)) {
		fprintf(stderr, "Cannot stat binary file %s\n",
			binarye->binary.file);
		goto err_close;
	}

	*cur += sizeof(struct opt_hdr_v1);

	args = (uint32_t *)*cur;
	*args = cpu_to_le32(binarye->binary.nargs);
	args++;
	for (argi = 0; argi < binarye->binary.nargs; argi++)
		args[argi] = cpu_to_le32(binarye->binary.args[argi]);

	*cur += (binarye->binary.nargs + 1) * sizeof(uint32_t);

	/*
	 * ARM executable code inside the BIN header on some mvebu platforms
	 * (e.g. A370, AXP) must always be aligned with the 128-bit boundary.
	 * This requirement can be met by inserting dummy arguments into
	 * BIN header, if needed.
	 */
	offset = *cur - (uint8_t *)main_hdr;
	add_args = ((16 - offset % 16) % 16) / sizeof(uint32_t);
	if (add_args) {
		*(args - 1) = cpu_to_le32(binarye->binary.nargs + add_args);
		*cur += add_args * sizeof(uint32_t);
	}

	ret = fread(*cur, s.st_size, 1, bin);
	if (ret != 1) {
		fprintf(stderr,
			"Could not read binary image %s\n",
			binarye->binary.file);
		goto err_close;
	}

	fclose(bin);

	*cur += ALIGN(s.st_size, 4);

	*((uint32_t *)*cur) = 0x00000000;
	**next_ext = 1;
	*next_ext = *cur;

	*cur += sizeof(uint32_t);

	binhdrsz = sizeof(struct opt_hdr_v1) +
		(binarye->binary.nargs + add_args + 2) * sizeof(uint32_t) +
		ALIGN(s.st_size, 4);
	hdr->headersz_lsb = cpu_to_le16(binhdrsz & 0xFFFF);
	hdr->headersz_msb = (binhdrsz & 0xFFFF0000) >> 16;

	return 0;

err_close:
	fclose(bin);

	return -1;
}

int export_pub_kak_hash(RSA *kak, struct secure_hdr_v1 *secure_hdr)
{
	FILE *hashf;
	int res;

	hashf = fopen("pub_kak_hash.txt", "w");
	if (!hashf) {
		fprintf(stderr, "Couldn't open hash file: '%s': %s\n",
			"pub_kak_hash.txt", strerror(errno));
		return 1;
	}

	res = kwb_export_pubkey(kak, &secure_hdr->kak, hashf, "KAK");

	fclose(hashf);

	return res < 0 ? 1 : 0;
}

int kwb_sign_csk_with_kak(struct image_tool_params *params,
			  struct secure_hdr_v1 *secure_hdr, RSA *csk)
{
	RSA *kak = NULL;
	RSA *kak_pub = NULL;
	int csk_idx = image_get_csk_index();
	struct sig_v1 tmp_sig;

	if (csk_idx < 0 || csk_idx > 15) {
		fprintf(stderr, "Invalid CSK index %d\n", csk_idx);
		return 1;
	}

	if (kwb_load_kak(params, &kak) < 0)
		return 1;

	if (export_pub_kak_hash(kak, secure_hdr))
		return 1;

	if (kwb_import_pubkey(&kak_pub, &secure_hdr->kak, "KAK") < 0)
		return 1;

	if (kwb_export_pubkey(csk, &secure_hdr->csk[csk_idx], NULL, "CSK") < 0)
		return 1;

	if (kwb_sign_and_verify(kak, &secure_hdr->csk,
				sizeof(secure_hdr->csk) +
				sizeof(secure_hdr->csksig),
				&tmp_sig, "CSK") < 0)
		return 1;

	if (kwb_verify(kak_pub, &secure_hdr->csk,
		       sizeof(secure_hdr->csk) +
		       sizeof(secure_hdr->csksig),
		       &tmp_sig, "CSK (2)") < 0)
		return 1;

	secure_hdr->csksig = tmp_sig;

	return 0;
}

int add_secure_header_v1(struct image_tool_params *params, uint8_t *ptr,
			 int payloadsz, size_t headersz, uint8_t *image,
			 struct secure_hdr_v1 *secure_hdr)
{
	struct image_cfg_element *e_jtagdelay;
	struct image_cfg_element *e_boxid;
	struct image_cfg_element *e_flashid;
	RSA *csk = NULL;
	unsigned char *image_ptr;
	size_t image_size;
	struct sig_v1 tmp_sig;
	bool specialized_img = image_get_spezialized_img();

	kwb_msg("Create secure header content\n");

	e_jtagdelay = image_find_option(IMAGE_CFG_JTAG_DELAY);
	e_boxid = image_find_option(IMAGE_CFG_BOX_ID);
	e_flashid = image_find_option(IMAGE_CFG_FLASH_ID);

	if (kwb_load_csk(params, &csk) < 0)
		return 1;

	secure_hdr->headertype = OPT_HDR_V1_SECURE_TYPE;
	secure_hdr->headersz_msb = 0;
	secure_hdr->headersz_lsb = cpu_to_le16(sizeof(struct secure_hdr_v1));
	if (e_jtagdelay)
		secure_hdr->jtag_delay = e_jtagdelay->jtag_delay;
	if (e_boxid && specialized_img)
		secure_hdr->boxid = cpu_to_le32(e_boxid->boxid);
	if (e_flashid && specialized_img)
		secure_hdr->flashid = cpu_to_le32(e_flashid->flashid);

	if (kwb_sign_csk_with_kak(params, secure_hdr, csk))
		return 1;

	image_ptr = ptr + headersz;
	image_size = payloadsz - headersz;

	if (kwb_sign_and_verify(csk, image_ptr, image_size,
				&secure_hdr->imgsig, "image") < 0)
		return 1;

	if (kwb_sign_and_verify(csk, image, headersz, &tmp_sig, "header") < 0)
		return 1;

	secure_hdr->hdrsig = tmp_sig;

	kwb_dump_fuse_cmds(secure_hdr);

	return 0;
}

static void *image_create_v1(size_t *imagesz, struct image_tool_params *params,
			     uint8_t *ptr, int payloadsz)
{
	struct image_cfg_element *e;
	struct main_hdr_v1 *main_hdr;
	struct opt_hdr_v1 *ohdr;
	struct register_set_hdr_v1 *register_set_hdr;
	struct secure_hdr_v1 *secure_hdr = NULL;
	size_t headersz;
	uint8_t *image, *cur;
	int hasext = 0;
	uint8_t *next_ext = NULL;
	int cfgi, datai, size;

	/*
	 * Calculate the size of the header and the size of the
	 * payload
	 */
	headersz = image_headersz_v1(&hasext);
	if (headersz == 0)
		return NULL;

	image = malloc(headersz);
	if (!image) {
		fprintf(stderr, "Cannot allocate memory for image\n");
		return NULL;
	}

	memset(image, 0, headersz);

	main_hdr = (struct main_hdr_v1 *)image;
	cur = image;
	cur += sizeof(struct main_hdr_v1);
	next_ext = &main_hdr->ext;

	/* Fill the main header */
	main_hdr->blocksize    =
		cpu_to_le32(payloadsz);
	main_hdr->headersz_lsb = cpu_to_le16(headersz & 0xFFFF);
	main_hdr->headersz_msb = (headersz & 0xFFFF0000) >> 16;
	main_hdr->destaddr     = cpu_to_le32(params->addr);
	main_hdr->execaddr     = cpu_to_le32(params->ep);
	main_hdr->srcaddr      = cpu_to_le32(headersz);
	main_hdr->ext          = hasext;
	main_hdr->version      = 1;
	main_hdr->blockid      = image_get_bootfrom();

	e = image_find_option(IMAGE_CFG_NAND_BLKSZ);
	if (e)
		main_hdr->nandblocksize = e->nandblksz / (64 * 1024);
	e = image_find_option(IMAGE_CFG_NAND_PAGESZ);
	if (e)
		main_hdr->nandpagesize = cpu_to_le16(e->nandpagesz);
	e = image_find_option(IMAGE_CFG_NAND_BADBLK_LOCATION);
	if (e)
		main_hdr->nandbadblklocation = e->nandbadblklocation;
	e = image_find_option(IMAGE_CFG_BAUDRATE);
	if (e)
		main_hdr->options |= baudrate_to_option(e->baudrate);
	e = image_find_option(IMAGE_CFG_UART_PORT);
	if (e)
		main_hdr->options |= (e->uart_port & 3) << 3;
	e = image_find_option(IMAGE_CFG_UART_MPP);
	if (e)
		main_hdr->options |= (e->uart_mpp & 7) << 5;
	e = image_find_option(IMAGE_CFG_DEBUG);
	if (e)
		main_hdr->flags = e->debug ? 0x1 : 0;

	/*
	 * For SATA srcaddr is specified in number of sectors starting from
	 * sector 0. The main header is stored at sector number 1.
	 * This expects the sector size to be 512 bytes.
	 * Header size is already aligned.
	 */
	if (main_hdr->blockid == IBR_HDR_SATA_ID)
		main_hdr->srcaddr = cpu_to_le32(headersz / 512 + 1);

	/*
	 * For SDIO srcaddr is specified in number of sectors starting from
	 * sector 0. The main header is stored at sector number 0.
	 * This expects sector size to be 512 bytes.
	 * Header size is already aligned.
	 */
	if (main_hdr->blockid == IBR_HDR_SDIO_ID)
		main_hdr->srcaddr = cpu_to_le32(headersz / 512);

	/* For PCIe srcaddr is not used and must be set to 0xFFFFFFFF. */
	if (main_hdr->blockid == IBR_HDR_PEX_ID)
		main_hdr->srcaddr = cpu_to_le32(0xFFFFFFFF);

	if (image_get_csk_index() >= 0) {
		/*
		 * only reserve the space here; we fill the header later since
		 * we need the header to be complete to compute the signatures
		 */
		secure_hdr = (struct secure_hdr_v1 *)cur;
		cur += sizeof(struct secure_hdr_v1);
		*next_ext = 1;
		next_ext = &secure_hdr->next;
	}

	datai = 0;
	register_set_hdr = (struct register_set_hdr_v1 *)cur;
	for (cfgi = 0; cfgi < cfgn; cfgi++) {
		e = &image_cfg[cfgi];
		if (e->type != IMAGE_CFG_DATA &&
		    e->type != IMAGE_CFG_DATA_DELAY)
			continue;
		if (e->type == IMAGE_CFG_DATA_DELAY) {
			size = sizeof(struct register_set_hdr_v1) + 8 * datai + 4;
			register_set_hdr->headertype = OPT_HDR_V1_REGISTER_TYPE;
			register_set_hdr->headersz_lsb = cpu_to_le16(size & 0xFFFF);
			register_set_hdr->headersz_msb = size >> 16;
			register_set_hdr->data[datai].last_entry.delay = e->regdata_delay;
			cur += size;
			*next_ext = 1;
			next_ext = &register_set_hdr->data[datai].last_entry.next;
			datai = 0;
			continue;
		}
		register_set_hdr->data[datai].entry.address =
			cpu_to_le32(e->regdata.raddr);
		register_set_hdr->data[datai].entry.value =
			cpu_to_le32(e->regdata.rdata);
		datai++;
	}
	if (datai != 0) {
		size = sizeof(struct register_set_hdr_v1) + 8 * datai + 4;
		register_set_hdr->headertype = OPT_HDR_V1_REGISTER_TYPE;
		register_set_hdr->headersz_lsb = cpu_to_le16(size & 0xFFFF);
		register_set_hdr->headersz_msb = size >> 16;
		/* Set delay to the smallest possible value 1ms. */
		register_set_hdr->data[datai].last_entry.delay = 1;
		cur += size;
		*next_ext = 1;
		next_ext = &register_set_hdr->data[datai].last_entry.next;
	}

	for (cfgi = 0; cfgi < cfgn; cfgi++) {
		e = &image_cfg[cfgi];
		if (e->type != IMAGE_CFG_BINARY)
			continue;

		if (add_binary_header_v1(&cur, &next_ext, e, main_hdr))
			return NULL;
	}

	if (secure_hdr && add_secure_header_v1(params, ptr, payloadsz + headersz,
					       headersz, image, secure_hdr))
		return NULL;

	*imagesz = headersz;

	/* Fill the real header size without padding into the main header */
	headersz = sizeof(*main_hdr);
	for_each_opt_hdr_v1 (ohdr, main_hdr)
		headersz += opt_hdr_v1_size(ohdr);
	main_hdr->headersz_lsb = cpu_to_le16(headersz & 0xFFFF);
	main_hdr->headersz_msb = (headersz & 0xFFFF0000) >> 16;

	/* Calculate and set the header checksum */
	main_hdr->checksum = image_checksum8(main_hdr, headersz);

	return image;
}

int recognize_keyword(char *keyword)
{
	int kw_id;

	for (kw_id = 1; kw_id < IMAGE_CFG_COUNT; ++kw_id)
		if (!strcmp(keyword, id_strs[kw_id]))
			return kw_id;

	return 0;
}

static int image_create_config_parse_oneline(char *line,
					     struct image_cfg_element *el)
{
	char *keyword, *saveptr, *value1, *value2;
	char delimiters[] = " \t";
	int keyword_id, ret, argi;
	char *unknown_msg = "Ignoring unknown line '%s'\n";

	keyword = strtok_r(line, delimiters, &saveptr);
	keyword_id = recognize_keyword(keyword);

	if (!keyword_id) {
		fprintf(stderr, unknown_msg, line);
		return 0;
	}

	el->type = keyword_id;

	value1 = strtok_r(NULL, delimiters, &saveptr);

	if (!value1) {
		fprintf(stderr, "Parameter missing in line '%s'\n", line);
		return -1;
	}

	switch (keyword_id) {
	case IMAGE_CFG_VERSION:
		el->version = atoi(value1);
		break;
	case IMAGE_CFG_BOOT_FROM:
		ret = image_boot_mode_id(value1);

		if (ret < 0) {
			fprintf(stderr, "Invalid boot media '%s'\n", value1);
			return -1;
		}
		el->bootfrom = ret;
		break;
	case IMAGE_CFG_NAND_BLKSZ:
		el->nandblksz = strtoul(value1, NULL, 16);
		break;
	case IMAGE_CFG_NAND_BADBLK_LOCATION:
		el->nandbadblklocation = strtoul(value1, NULL, 16);
		break;
	case IMAGE_CFG_NAND_ECC_MODE:
		ret = image_nand_ecc_mode_id(value1);

		if (ret < 0) {
			fprintf(stderr, "Invalid NAND ECC mode '%s'\n", value1);
			return -1;
		}
		el->nandeccmode = ret;
		break;
	case IMAGE_CFG_NAND_PAGESZ:
		el->nandpagesz = strtoul(value1, NULL, 16);
		break;
	case IMAGE_CFG_BINARY:
		argi = 0;

		el->binary.file = strdup(value1);
		while (1) {
			char *value = strtok_r(NULL, delimiters, &saveptr);

			if (!value)
				break;
			el->binary.args[argi] = strtoul(value, NULL, 16);
			argi++;
			if (argi >= BINARY_MAX_ARGS) {
				fprintf(stderr,
					"Too many arguments for BINARY\n");
				return -1;
			}
		}
		el->binary.nargs = argi;
		break;
	case IMAGE_CFG_DATA:
		value2 = strtok_r(NULL, delimiters, &saveptr);

		if (!value1 || !value2) {
			fprintf(stderr,
				"Invalid number of arguments for DATA\n");
			return -1;
		}

		el->regdata.raddr = strtoul(value1, NULL, 16);
		el->regdata.rdata = strtoul(value2, NULL, 16);
		break;
	case IMAGE_CFG_DATA_DELAY:
		if (!strcmp(value1, "SDRAM_SETUP"))
			el->regdata_delay = REGISTER_SET_HDR_OPT_DELAY_SDRAM_SETUP;
		else
			el->regdata_delay = REGISTER_SET_HDR_OPT_DELAY_MS(strtoul(value1, NULL, 10));
		break;
	case IMAGE_CFG_BAUDRATE:
		el->baudrate = strtoul(value1, NULL, 10);
		break;
	case IMAGE_CFG_UART_PORT:
		el->uart_port = strtoul(value1, NULL, 16);
		break;
	case IMAGE_CFG_UART_MPP:
		el->uart_mpp = strtoul(value1, NULL, 16);
		break;
	case IMAGE_CFG_DEBUG:
		el->debug = strtoul(value1, NULL, 10);
		break;
	case IMAGE_CFG_KAK:
		el->key_name = strdup(value1);
		break;
	case IMAGE_CFG_CSK:
		el->key_name = strdup(value1);
		break;
	case IMAGE_CFG_CSK_INDEX:
		el->csk_idx = strtol(value1, NULL, 0);
		break;
	case IMAGE_CFG_JTAG_DELAY:
		el->jtag_delay = strtoul(value1, NULL, 0);
		break;
	case IMAGE_CFG_BOX_ID:
		el->boxid = strtoul(value1, NULL, 0);
		break;
	case IMAGE_CFG_FLASH_ID:
		el->flashid = strtoul(value1, NULL, 0);
		break;
	case IMAGE_CFG_SEC_SPECIALIZED_IMG:
		el->sec_specialized_img = true;
		break;
	case IMAGE_CFG_SEC_COMMON_IMG:
		el->sec_specialized_img = false;
		break;
	case IMAGE_CFG_SEC_BOOT_DEV:
		el->sec_boot_dev = strtoul(value1, NULL, 0);
		break;
	case IMAGE_CFG_SEC_FUSE_DUMP:
		el->name = strdup(value1);
		break;
	default:
		fprintf(stderr, unknown_msg, line);
	}

	return 0;
}

/*
 * Parse the configuration file 'fcfg' into the array of configuration
 * elements 'image_cfg', and return the number of configuration
 * elements in 'cfgn'.
 */
static int image_create_config_parse(FILE *fcfg)
{
	int ret;
	int cfgi = 0;

	/* Parse the configuration file */
	while (!feof(fcfg)) {
		char *line;
		char buf[256];

		/* Read the current line */
		memset(buf, 0, sizeof(buf));
		line = fgets(buf, sizeof(buf), fcfg);
		if (!line)
			break;

		/* Ignore useless lines */
		if (line[0] == '\n' || line[0] == '#')
			continue;

		/* Strip final newline */
		if (line[strlen(line) - 1] == '\n')
			line[strlen(line) - 1] = 0;

		/* Parse the current line */
		ret = image_create_config_parse_oneline(line,
							&image_cfg[cfgi]);
		if (ret)
			return ret;

		cfgi++;

		if (cfgi >= IMAGE_CFG_ELEMENT_MAX) {
			fprintf(stderr,
				"Too many configuration elements in .cfg file\n");
			return -1;
		}
	}

	cfgn = cfgi;
	return 0;
}

static int image_get_version(void)
{
	struct image_cfg_element *e;

	e = image_find_option(IMAGE_CFG_VERSION);
	if (!e)
		return -1;

	return e->version;
}

static void kwbimage_set_header(void *ptr, struct stat *sbuf, int ifd,
				struct image_tool_params *params)
{
	FILE *fcfg;
	void *image = NULL;
	int version;
	size_t headersz = 0;
	size_t datasz;
	uint32_t checksum;
	struct stat s;
	int ret;

	/*
	 * Do not use sbuf->st_size as it contains size with padding.
	 * We need original image data size, so stat original file.
	 */
	if (stat(params->datafile, &s)) {
		fprintf(stderr, "Could not stat data file %s: %s\n",
			params->datafile, strerror(errno));
		exit(EXIT_FAILURE);
	}
	datasz = ALIGN(s.st_size, 4);

	fcfg = fopen(params->imagename, "r");
	if (!fcfg) {
		fprintf(stderr, "Could not open input file %s\n",
			params->imagename);
		exit(EXIT_FAILURE);
	}

	image_cfg = malloc(IMAGE_CFG_ELEMENT_MAX *
			   sizeof(struct image_cfg_element));
	if (!image_cfg) {
		fprintf(stderr, "Cannot allocate memory\n");
		fclose(fcfg);
		exit(EXIT_FAILURE);
	}

	memset(image_cfg, 0,
	       IMAGE_CFG_ELEMENT_MAX * sizeof(struct image_cfg_element));
	rewind(fcfg);

	ret = image_create_config_parse(fcfg);
	fclose(fcfg);
	if (ret) {
		free(image_cfg);
		exit(EXIT_FAILURE);
	}

	version = image_get_version();
	switch (version) {
		/*
		 * Fallback to version 0 if no version is provided in the
		 * cfg file
		 */
	case -1:
	case 0:
		image = image_create_v0(&headersz, params, datasz + 4);
		break;

	case 1:
		image = image_create_v1(&headersz, params, ptr, datasz + 4);
		break;

	default:
		fprintf(stderr, "Unsupported version %d\n", version);
		free(image_cfg);
		exit(EXIT_FAILURE);
	}

	if (!image) {
		fprintf(stderr, "Could not create image\n");
		free(image_cfg);
		exit(EXIT_FAILURE);
	}

	free(image_cfg);

	/* Build and add image data checksum */
	checksum = cpu_to_le32(image_checksum32((uint8_t *)ptr + headersz,
						datasz));
	memcpy((uint8_t *)ptr + headersz + datasz, &checksum, sizeof(uint32_t));

	/* Finally copy the header into the image area */
	memcpy(ptr, image, headersz);

	free(image);
}

static void kwbimage_print_header(const void *ptr)
{
	struct main_hdr_v0 *mhdr = (struct main_hdr_v0 *)ptr;
	struct opt_hdr_v1 *ohdr;

	printf("Image Type:   MVEBU Boot from %s Image\n",
	       image_boot_mode_name(mhdr->blockid));
	printf("Image version:%d\n", kwbimage_version(ptr));

	for_each_opt_hdr_v1 (ohdr, mhdr) {
		if (ohdr->headertype == OPT_HDR_V1_BINARY_TYPE) {
			printf("BIN Hdr Size: ");
			genimg_print_size(opt_hdr_v1_size(ohdr) - 12 -
					  4 * ohdr->data[0]);
		}
	}

	printf("Data Size:    ");
	genimg_print_size(mhdr->blocksize - sizeof(uint32_t));
	printf("Load Address: %08x\n", mhdr->destaddr);
	printf("Entry Point:  %08x\n", mhdr->execaddr);
}

static int kwbimage_check_image_types(uint8_t type)
{
	if (type == IH_TYPE_KWBIMAGE)
		return EXIT_SUCCESS;

	return EXIT_FAILURE;
}

static int kwbimage_verify_header(unsigned char *ptr, int image_size,
				  struct image_tool_params *params)
{
	size_t header_size = kwbheader_size(ptr);
	uint8_t blockid;
	uint32_t offset;
	uint32_t size;
	uint8_t csum;

	if (header_size > image_size)
		return -FDT_ERR_BADSTRUCTURE;

	if (!main_hdr_checksum_ok(ptr))
		return -FDT_ERR_BADSTRUCTURE;

	/* Only version 0 extended header has checksum */
	if (kwbimage_version(ptr) == 0) {
		struct main_hdr_v0 *mhdr = (struct main_hdr_v0 *)ptr;

		if (mhdr->ext & 0x1) {
			struct ext_hdr_v0 *ext_hdr = (void *)(mhdr + 1);

			csum = image_checksum8(ext_hdr, sizeof(*ext_hdr) - 1);
			if (csum != ext_hdr->checksum)
				return -FDT_ERR_BADSTRUCTURE;
		}

		blockid = mhdr->blockid;
		offset = le32_to_cpu(mhdr->srcaddr);
		size = le32_to_cpu(mhdr->blocksize);
	} else if (kwbimage_version(ptr) == 1) {
		struct main_hdr_v1 *mhdr = (struct main_hdr_v1 *)ptr;
		const uint8_t *mhdr_end;
		struct opt_hdr_v1 *ohdr;

		mhdr_end = (uint8_t *)mhdr + header_size;
		for_each_opt_hdr_v1 (ohdr, ptr)
			if (!opt_hdr_v1_valid_size(ohdr, mhdr_end))
				return -FDT_ERR_BADSTRUCTURE;

		blockid = mhdr->blockid;
		offset = le32_to_cpu(mhdr->srcaddr);
		size = le32_to_cpu(mhdr->blocksize);
	} else {
		return -FDT_ERR_BADSTRUCTURE;
	}

	/*
	 * For SATA srcaddr is specified in number of sectors.
	 * The main header is must be stored at sector number 1.
	 * This expects that sector size is 512 bytes and recalculates
	 * data offset to bytes relative to the main header.
	 */
	if (blockid == IBR_HDR_SATA_ID) {
		if (offset < 1)
			return -FDT_ERR_BADSTRUCTURE;
		offset -= 1;
		offset *= 512;
	}

	/*
	 * For SDIO srcaddr is specified in number of sectors.
	 * This expects that sector size is 512 bytes and recalculates
	 * data offset to bytes.
	 */
	if (blockid == IBR_HDR_SDIO_ID)
		offset *= 512;

	/*
	 * For PCIe srcaddr is always set to 0xFFFFFFFF.
	 * This expects that data starts after all headers.
	 */
	if (blockid == IBR_HDR_PEX_ID && offset == 0xFFFFFFFF)
		offset = header_size;

	if (offset > image_size || offset % 4 != 0)
		return -FDT_ERR_BADSTRUCTURE;

	if (size < 4 || offset + size > image_size || size % 4 != 0)
		return -FDT_ERR_BADSTRUCTURE;

	if (image_checksum32(ptr + offset, size - 4) !=
	    *(uint32_t *)(ptr + offset + size - 4))
		return -FDT_ERR_BADSTRUCTURE;

	return 0;
}

static int kwbimage_generate(struct image_tool_params *params,
			     struct image_type_params *tparams)
{
	FILE *fcfg;
	struct stat s;
	int alloc_len;
	int bootfrom;
	int version;
	void *hdr;
	int ret;

	fcfg = fopen(params->imagename, "r");
	if (!fcfg) {
		fprintf(stderr, "Could not open input file %s\n",
			params->imagename);
		exit(EXIT_FAILURE);
	}

	if (stat(params->datafile, &s)) {
		fprintf(stderr, "Could not stat data file %s: %s\n",
			params->datafile, strerror(errno));
		exit(EXIT_FAILURE);
	}

	image_cfg = malloc(IMAGE_CFG_ELEMENT_MAX *
			   sizeof(struct image_cfg_element));
	if (!image_cfg) {
		fprintf(stderr, "Cannot allocate memory\n");
		fclose(fcfg);
		exit(EXIT_FAILURE);
	}

	memset(image_cfg, 0,
	       IMAGE_CFG_ELEMENT_MAX * sizeof(struct image_cfg_element));
	rewind(fcfg);

	ret = image_create_config_parse(fcfg);
	fclose(fcfg);
	if (ret) {
		free(image_cfg);
		exit(EXIT_FAILURE);
	}

	bootfrom = image_get_bootfrom();
	version = image_get_version();
	switch (version) {
		/*
		 * Fallback to version 0 if no version is provided in the
		 * cfg file
		 */
	case -1:
	case 0:
		alloc_len = image_headersz_v0(NULL);
		break;

	case 1:
		alloc_len = image_headersz_v1(NULL);
		break;

	default:
		fprintf(stderr, "Unsupported version %d\n", version);
		free(image_cfg);
		exit(EXIT_FAILURE);
	}

	free(image_cfg);

	hdr = malloc(alloc_len);
	if (!hdr) {
		fprintf(stderr, "%s: malloc return failure: %s\n",
			params->cmdname, strerror(errno));
		exit(EXIT_FAILURE);
	}

	memset(hdr, 0, alloc_len);
	tparams->header_size = alloc_len;
	tparams->hdr = hdr;

	/*
	 * The resulting image needs to be 4-byte aligned. At least
	 * the Marvell hdrparser tool complains if its unaligned.
	 * After the image data is stored 4-byte checksum.
	 * Final UART image must be aligned to 128 bytes.
	 * Final SPI and NAND images must be aligned to 256 bytes.
	 * Final SATA and SDIO images must be aligned to 512 bytes.
	 */
	if (bootfrom == IBR_HDR_SPI_ID || bootfrom == IBR_HDR_NAND_ID)
		return 4 + (256 - (alloc_len + s.st_size + 4) % 256) % 256;
	else if (bootfrom == IBR_HDR_SATA_ID || bootfrom == IBR_HDR_SDIO_ID)
		return 4 + (512 - (alloc_len + s.st_size + 4) % 512) % 512;
	else if (bootfrom == IBR_HDR_UART_ID)
		return 4 + (128 - (alloc_len + s.st_size + 4) % 128) % 128;
	else
		return 4 + (4 - s.st_size % 4) % 4;
}

static int kwbimage_extract_subimage(void *ptr, struct image_tool_params *params)
{
	struct main_hdr_v1 *mhdr = (struct main_hdr_v1 *)ptr;
	size_t header_size = kwbheader_size(ptr);
	struct opt_hdr_v1 *ohdr;
	int idx = params->pflag;
	int cur_idx = 0;
	uint32_t offset;
	ulong image;
	ulong size;

	for_each_opt_hdr_v1 (ohdr, ptr) {
		if (ohdr->headertype != OPT_HDR_V1_BINARY_TYPE)
			continue;

		if (idx == cur_idx) {
			image = (ulong)&ohdr->data[4 + 4 * ohdr->data[0]];
			size = opt_hdr_v1_size(ohdr) - 12 - 4 * ohdr->data[0];
			goto extract;
		}

		++cur_idx;
	}

	if (idx != cur_idx) {
		printf("Image %d is not present\n", idx);
		return -1;
	}

	offset = le32_to_cpu(mhdr->srcaddr);

	if (mhdr->blockid == IBR_HDR_SATA_ID) {
		offset -= 1;
		offset *= 512;
	}

	if (mhdr->blockid == IBR_HDR_SDIO_ID)
		offset *= 512;

	if (mhdr->blockid == IBR_HDR_PEX_ID && offset == 0xFFFFFFFF)
		offset = header_size;

	image = (ulong)((uint8_t *)ptr + offset);
	size = le32_to_cpu(mhdr->blocksize) - 4;

extract:
	return imagetool_save_subimage(params->outfile, image, size);
}

/*
 * Report Error if xflag is set in addition to default
 */
static int kwbimage_check_params(struct image_tool_params *params)
{
	if (!params->iflag && (!params->imagename || !strlen(params->imagename))) {
		char *msg = "Configuration file for kwbimage creation omitted";

		fprintf(stderr, "Error:%s - %s\n", params->cmdname, msg);
		return 1;
	}

	return (params->dflag && (params->fflag || params->lflag)) ||
		(params->fflag && (params->dflag || params->lflag)) ||
		(params->lflag && (params->dflag || params->fflag)) ||
		(params->xflag);
}

/*
 * kwbimage type parameters definition
 */
U_BOOT_IMAGE_TYPE(
	kwbimage,
	"Marvell MVEBU Boot Image support",
	0,
	NULL,
	kwbimage_check_params,
	kwbimage_verify_header,
	kwbimage_print_header,
	kwbimage_set_header,
	kwbimage_extract_subimage,
	kwbimage_check_image_types,
	NULL,
	kwbimage_generate
);
