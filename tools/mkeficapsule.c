// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright 2018 Linaro Limited
 *		Author: AKASHI Takahiro
 */

#include <errno.h>
#include <getopt.h>
#include <malloc.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <linux/types.h>

#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/types.h>

#include <linux/kconfig.h>
#if IS_ENABLED(CONFIG_EFI_CAPSULE_AUTHENTICATE)
#include <openssl/asn1.h>
#include <openssl/bio.h>
#include <openssl/evp.h>
#include <openssl/err.h>
#include <openssl/pem.h>
#include <openssl/pkcs7.h>
#endif

#include <linux/libfdt.h>

typedef __u8 u8;
typedef __u16 u16;
typedef __u32 u32;
typedef __u64 u64;
typedef __s16 s16;
typedef __s32 s32;

#define aligned_u64 __aligned_u64

#define SIGNATURE_NODENAME	"signature"
#define OVERLAY_NODENAME	"__overlay__"

#ifndef __packed
#define __packed __attribute__((packed))
#endif

#include <efi.h>
#include <efi_api.h>

static const char *tool_name = "mkeficapsule";

efi_guid_t efi_guid_fm_capsule = EFI_FIRMWARE_MANAGEMENT_CAPSULE_ID_GUID;
efi_guid_t efi_guid_image_type_uboot_fit =
		EFI_FIRMWARE_IMAGE_TYPE_UBOOT_FIT_GUID;
efi_guid_t efi_guid_image_type_uboot_raw =
		EFI_FIRMWARE_IMAGE_TYPE_UBOOT_RAW_GUID;
efi_guid_t efi_guid_cert_type_pkcs7 = EFI_CERT_TYPE_PKCS7_GUID;

#if IS_ENABLED(CONFIG_EFI_CAPSULE_AUTHENTICATE)
static const char *opts_short = "f:r:i:I:v:D:K:P:C:m:dOh";
#else
static const char *opts_short = "f:r:i:I:v:D:K:Oh";
#endif

static struct option options[] = {
	{"fit", required_argument, NULL, 'f'},
	{"raw", required_argument, NULL, 'r'},
	{"index", required_argument, NULL, 'i'},
	{"instance", required_argument, NULL, 'I'},
	{"dtb", required_argument, NULL, 'D'},
	{"public key", required_argument, NULL, 'K'},
#if IS_ENABLED(CONFIG_EFI_CAPSULE_AUTHENTICATE)
	{"private-key", required_argument, NULL, 'P'},
	{"certificate", required_argument, NULL, 'C'},
	{"monotonic-count", required_argument, NULL, 'm'},
	{"dump-sig", no_argument, NULL, 'd'},
#endif
	{"overlay", no_argument, NULL, 'O'},
	{"help", no_argument, NULL, 'h'},
	{NULL, 0, NULL, 0},
};

static void print_usage(void)
{
	printf("Usage: %s [options] <output file>\n"
	       "Options:\n"

	       "\t-f, --fit <fit image>       new FIT image file\n"
	       "\t-r, --raw <raw image>       new raw image file\n"
	       "\t-i, --index <index>         update image index\n"
	       "\t-I, --instance <instance>   update hardware instance\n"
	       "\t-K, --public-key <key file> public key esl file\n"
	       "\t-D, --dtb <dtb file>        dtb file\n"
#if IS_ENABLED(CONFIG_EFI_CAPSULE_AUTHENTICATE)
	       "\t-P, --private-key <privkey file>  private key file\n"
	       "\t-C, --certificate <cert file>     signer's certificate file\n"
	       "\t-m, --monotonic-count <count>     monotonic count\n"
	       "\t-d, --dump_sig              dump signature (*.p7)\n"
#endif
	       "\t-O, --overlay               the dtb file is an overlay\n"
	       "\t-h, --help                  print a help message\n",
	       tool_name);
}

static int fdt_add_pub_key_data(void *sptr, void *dptr, size_t key_size,
				bool overlay)
{
	int parent;
	int ov_node;
	int frag_node;
	int ret = 0;

	if (overlay) {
		/*
		 * The signature would be stored in the
		 * first fragment node of the overlay
		 */
		frag_node = fdt_first_subnode(dptr, 0);
		if (frag_node == -FDT_ERR_NOTFOUND) {
			fprintf(stderr,
				"Couldn't find the fragment node: %s\n",
				fdt_strerror(frag_node));
			goto done;
		}

		ov_node = fdt_subnode_offset(dptr, frag_node, OVERLAY_NODENAME);
		if (ov_node == -FDT_ERR_NOTFOUND) {
			fprintf(stderr,
				"Couldn't find the __overlay__ node: %s\n",
				fdt_strerror(ov_node));
			goto done;
		}
	} else {
		ov_node = 0;
	}

	parent = fdt_subnode_offset(dptr, ov_node, SIGNATURE_NODENAME);
	if (parent == -FDT_ERR_NOTFOUND) {
		parent = fdt_add_subnode(dptr, ov_node, SIGNATURE_NODENAME);
		if (parent < 0) {
			ret = parent;
			if (ret != -FDT_ERR_NOSPACE) {
				fprintf(stderr,
					"Couldn't create signature node: %s\n",
					fdt_strerror(parent));
			}
		}
	}
	if (ret)
		goto done;

	/* Write the key to the FDT node */
	ret = fdt_setprop(dptr, parent, "capsule-key",
			  sptr, key_size);

done:
	if (ret)
		ret = ret == -FDT_ERR_NOSPACE ? -ENOSPC : -EIO;

	return ret;
}

static int add_public_key(const char *pkey_file, const char *dtb_file,
			  bool overlay)
{
	int ret;
	int srcfd = -1;
	int destfd = -1;
	void *sptr = NULL;
	void *dptr = NULL;
	off_t src_size;
	struct stat pub_key;
	struct stat dtb;

	/* Find out the size of the public key */
	srcfd = open(pkey_file, O_RDONLY);
	if (srcfd == -1) {
		fprintf(stderr, "%s: Can't open %s: %s\n",
			__func__, pkey_file, strerror(errno));
		ret = -1;
		goto err;
	}

	ret = fstat(srcfd, &pub_key);
	if (ret == -1) {
		fprintf(stderr, "%s: Can't stat %s: %s\n",
			__func__, pkey_file, strerror(errno));
		ret = -1;
		goto err;
	}

	src_size = pub_key.st_size;

	/* mmap the public key esl file */
	sptr = mmap(0, src_size, PROT_READ, MAP_SHARED, srcfd, 0);
	if (sptr == MAP_FAILED) {
		fprintf(stderr, "%s: Failed to mmap %s:%s\n",
			__func__, pkey_file, strerror(errno));
		ret = -1;
		goto err;
	}

	/* Open the dest FDT */
	destfd = open(dtb_file, O_RDWR);
	if (destfd == -1) {
		fprintf(stderr, "%s: Can't open %s: %s\n",
			__func__, dtb_file, strerror(errno));
		ret = -1;
		goto err;
	}

	ret = fstat(destfd, &dtb);
	if (ret == -1) {
		fprintf(stderr, "%s: Can't stat %s: %s\n",
			__func__, dtb_file, strerror(errno));
		goto err;
	}

	dtb.st_size += src_size + 0x30;
	if (ftruncate(destfd, dtb.st_size)) {
		fprintf(stderr, "%s: Can't expand %s: %s\n",
			__func__, dtb_file, strerror(errno));
		ret = -1;
		goto err;
	}

	errno = 0;
	/* mmap the dtb file */
	dptr = mmap(0, dtb.st_size, PROT_READ | PROT_WRITE, MAP_SHARED,
		    destfd, 0);
	if (dptr == MAP_FAILED) {
		fprintf(stderr, "%s: Failed to mmap %s:%s\n",
			__func__, dtb_file, strerror(errno));
		ret = -1;
		goto err;
	}

	if (fdt_check_header(dptr)) {
		fprintf(stderr, "%s: Invalid FDT header\n", __func__);
		ret = -1;
		goto err;
	}

	ret = fdt_open_into(dptr, dptr, dtb.st_size);
	if (ret) {
		fprintf(stderr, "%s: Cannot expand FDT: %s\n",
			__func__, fdt_strerror(ret));
		ret = -1;
		goto err;
	}

	/* Copy the esl file to the expanded FDT */
	ret = fdt_add_pub_key_data(sptr, dptr, src_size, overlay);
	if (ret < 0) {
		fprintf(stderr, "%s: Unable to add public key to the FDT\n",
			__func__);
		ret = -1;
		goto err;
	}

	ret = 0;

err:
	if (sptr)
		munmap(sptr, src_size);

	if (dptr)
		munmap(dptr, dtb.st_size);

	if (srcfd != -1)
		close(srcfd);

	if (destfd != -1)
		close(destfd);

	return ret;
}

struct auth_context {
	char *key_file;
	char *cert_file;
	u8 *image_data;
	size_t image_size;
	struct efi_firmware_image_authentication auth;
	u8 *sig_data;
	size_t sig_size;
};

static int dump_sig;

#if IS_ENABLED(CONFIG_EFI_CAPSULE_AUTHENTICATE)
static EVP_PKEY *fileio_read_pkey(const char *filename)
{
	EVP_PKEY *key = NULL;
	BIO *bio;

	bio = BIO_new_file(filename, "r");
	if (!bio)
		goto out;

	key = PEM_read_bio_PrivateKey(bio, NULL, NULL, NULL);

out:
	BIO_free_all(bio);
	if (!key) {
		printf("Can't load key from file '%s'\n", filename);
		ERR_print_errors_fp(stderr);
	}

	return key;
}

static X509 *fileio_read_cert(const char *filename)
{
	X509 *cert = NULL;
	BIO *bio;

	bio = BIO_new_file(filename, "r");
	if (!bio)
		goto out;

	cert = PEM_read_bio_X509(bio, NULL, NULL, NULL);

out:
	BIO_free_all(bio);
	if (!cert) {
		printf("Can't load certificate from file '%s'\n", filename);
		ERR_print_errors_fp(stderr);
	}

	return cert;
}

static int create_auth_data(struct auth_context *ctx)
{
	EVP_PKEY *key = NULL;
	X509 *cert = NULL;
	BIO *data_bio = NULL;
	const EVP_MD *md;
	PKCS7 *p7;
	int ret = -1, flags;

        OpenSSL_add_all_digests();
        OpenSSL_add_all_ciphers();
        ERR_load_crypto_strings();

	key = fileio_read_pkey(ctx->key_file);
	if (!key)
		goto err;
	cert = fileio_read_cert(ctx->cert_file);
	if (!cert)
		goto err;

	/*
	 * create a BIO, containing:
	 *  * firmware image
	 *  * monotonic count
	 * in this order!
	 * See EDK2's FmpAuthenticatedHandlerRsa2048Sha256()
	 */
	data_bio = BIO_new(BIO_s_mem());
	BIO_write(data_bio, ctx->image_data, ctx->image_size);
	BIO_write(data_bio, &ctx->auth.monotonic_count,
		  sizeof(ctx->auth.monotonic_count));

	md = EVP_get_digestbyname("SHA256");
	if (!md)
		goto err;

	/* create signature */
	/* TODO: maybe add PKCS7_NOATTR and PKCS7_NOSMIMECAP */
	flags = PKCS7_BINARY | PKCS7_DETACHED;
        p7 = PKCS7_sign(NULL, NULL, NULL, data_bio, flags | PKCS7_PARTIAL);
	if (!p7)
		goto err;
        if (!PKCS7_sign_add_signer(p7, cert, key, md, flags))
		goto err;
        if (!PKCS7_final(p7, data_bio, flags))
		goto err;

	/* convert pkcs7 into DER */
	ctx->sig_data = NULL;
	ctx->sig_size = ASN1_item_i2d((ASN1_VALUE *)p7, &ctx->sig_data,
				      ASN1_ITEM_rptr(PKCS7));
	if (!ctx->sig_size)
		goto err;

	/* fill auth_info */
	ctx->auth.auth_info.hdr.dwLength = sizeof(ctx->auth.auth_info)
						+ ctx->sig_size;
	ctx->auth.auth_info.hdr.wRevision = WIN_CERT_REVISION_2_0;
	ctx->auth.auth_info.hdr.wCertificateType = WIN_CERT_TYPE_EFI_GUID;
	memcpy(&ctx->auth.auth_info.cert_type, &efi_guid_cert_type_pkcs7,
	       sizeof(efi_guid_cert_type_pkcs7));

	ret = 0;
err:
	BIO_free_all(data_bio);
	EVP_PKEY_free(key);
	X509_free(cert);

	return ret;
}

static int dump_signature(const char *path, u8 *signature, size_t sig_size) {
	char *sig_path;
	FILE *f;
	size_t size;
	int ret = -1;

	sig_path = malloc(strlen(path) + 3 + 1);
	if (!sig_path)
		return ret;

	sprintf(sig_path, "%s.p7", path);
	f = fopen(sig_path, "w");
	if (!f)
		goto err;

	size = fwrite(signature, 1, sig_size, f);
	if (size == sig_size)
		ret = 0;

	fclose(f);
err:
	free(sig_path);
	return ret;
}
#endif

static int create_fwbin(char *path, char *bin, efi_guid_t *guid,
			unsigned long index, unsigned long instance,
			uint64_t mcount, char *privkey_file, char *cert_file)
{
	struct efi_capsule_header header;
	struct efi_firmware_management_capsule_header capsule;
	struct efi_firmware_management_capsule_image_header image;
	struct auth_context auth_context;
	FILE *f, *g;
	struct stat bin_stat;
	u8 *data;
	size_t size;
	u64 offset;

#ifdef DEBUG
	printf("For output: %s\n", path);
	printf("\tbin: %s\n\ttype: %pUl\n", bin, guid);
	printf("\tindex: %ld\n\tinstance: %ld\n", index, instance);
#endif
	auth_context.sig_size = 0;

	g = fopen(bin, "r");
	if (!g) {
		printf("cannot open %s\n", bin);
		return -1;
	}
	if (stat(bin, &bin_stat) < 0) {
		printf("cannot determine the size of %s\n", bin);
		goto err_1;
	}
	data = malloc(bin_stat.st_size);
	if (!data) {
		printf("cannot allocate memory: %zx\n", (size_t)bin_stat.st_size);
		goto err_1;
	}

	size = fread(data, 1, bin_stat.st_size, g);
	if (size < bin_stat.st_size) {
		printf("read failed (%zx)\n", size);
		goto err_2;
	}

	/* first, calculate signature to determine its size */
#if IS_ENABLED(CONFIG_EFI_CAPSULE_AUTHENTICATE)
	if (privkey_file && cert_file) {
		auth_context.key_file = privkey_file;
		auth_context.cert_file = cert_file;
		auth_context.auth.monotonic_count = mcount;
		auth_context.image_data = data;
		auth_context.image_size = bin_stat.st_size;

		if (create_auth_data(&auth_context)) {
			printf("Signing firmware image failed\n");
			goto err_3;
		}

		if (dump_sig &&
		    dump_signature(path, auth_context.sig_data,
				   auth_context.sig_size)) {
			printf("Creating signature file failed\n");
			goto err_3;
		}
	}
#endif

	header.capsule_guid = efi_guid_fm_capsule;
	header.header_size = sizeof(header);
	/* TODO: The current implementation ignores flags */
	header.flags = CAPSULE_FLAGS_PERSIST_ACROSS_RESET;
	header.capsule_image_size = sizeof(header)
					+ sizeof(capsule) + sizeof(u64)
					+ sizeof(image)
					+ bin_stat.st_size;
	if (auth_context.sig_size)
		header.capsule_image_size += sizeof(auth_context.auth)
				+ auth_context.sig_size;

	f = fopen(path, "w");
	if (!f) {
		printf("cannot open %s\n", path);
		goto err_3;
	}

	size = fwrite(&header, 1, sizeof(header), f);
	if (size < sizeof(header)) {
		printf("write failed (%zx)\n", size);
		goto err_4;
	}

	capsule.version = 0x00000001;
	capsule.embedded_driver_count = 0;
	capsule.payload_item_count = 1;
	size = fwrite(&capsule, 1, sizeof(capsule), f);
	if (size < (sizeof(capsule))) {
		printf("write failed (%zx)\n", size);
		goto err_4;
	}
	offset = sizeof(capsule) + sizeof(u64);
	size = fwrite(&offset, 1, sizeof(offset), f);
	if (size < sizeof(offset)) {
		printf("write failed (%zx)\n", size);
		goto err_4;
	}

	image.version = 0x00000003;
	memcpy(&image.update_image_type_id, guid, sizeof(*guid));
	image.update_image_index = index;
	image.reserved[0] = 0;
	image.reserved[1] = 0;
	image.reserved[2] = 0;
	image.update_image_size = bin_stat.st_size;
	if (auth_context.sig_size)
		image.update_image_size += sizeof(auth_context.auth)
				+ auth_context.sig_size;
	image.update_vendor_code_size = 0; /* none */
	image.update_hardware_instance = instance;
	image.image_capsule_support = 0;

	size = fwrite(&image, 1, sizeof(image), f);
	if (size < sizeof(image)) {
		printf("write failed (%zx)\n", size);
		goto err_4;
	}

#if IS_ENABLED(CONFIG_EFI_CAPSULE_AUTHENTICATE)
	if (auth_context.sig_size) {
		size = fwrite(&auth_context.auth, 1,
			      sizeof(auth_context.auth), f);
		if (size < sizeof(auth_context.auth)) {
			printf("write failed (%zx)\n", size);
			goto err_4;
		}
		size = fwrite(auth_context.sig_data, 1,
			      auth_context.sig_size, f);
		if (size < auth_context.sig_size) {
			printf("write failed (%zx)\n", size);
			goto err_4;
		}
	}
#endif

	size = fwrite(data, 1, bin_stat.st_size, f);
	if (size < bin_stat.st_size) {
		printf("write failed (%zx)\n", size);
		goto err_4;
	}

	fclose(f);
	fclose(g);
	free(data);
#if IS_ENABLED(CONFIG_EFI_CAPSULE_AUTHENTICATE)
	if (auth_context.sig_size)
		OPENSSL_free(auth_context.sig_data);
#endif

	return 0;

err_4:
	fclose(f);
err_3:
#if IS_ENABLED(CONFIG_EFI_CAPSULE_AUTHENTICATE)
	if (auth_context.sig_size)
		OPENSSL_free(auth_context.sig_data);
#endif
err_2:
	free(data);
err_1:
	fclose(g);

	return -1;
}

/*
 * Usage:
 *   $ mkeficapsule -f <firmware binary> <output file>
 */
int main(int argc, char **argv)
{
	char *file;
	char *pkey_file;
	char *dtb_file;
	efi_guid_t *guid;
	unsigned long index, instance;
	uint64_t mcount;
	char *privkey_file, *cert_file;
	int c, idx;
	int ret;
	bool overlay = false;

	file = NULL;
	pkey_file = NULL;
	dtb_file = NULL;
	guid = NULL;
	index = 0;
	instance = 0;
	mcount = 0;
	privkey_file = NULL;
	cert_file = NULL;
	dump_sig = 0;
	for (;;) {
		c = getopt_long(argc, argv, opts_short, options, &idx);
		if (c == -1)
			break;

		switch (c) {
		case 'f':
			if (file) {
				printf("Image already specified\n");
				return -1;
			}
			file = optarg;
			guid = &efi_guid_image_type_uboot_fit;
			break;
		case 'r':
			if (file) {
				printf("Image already specified\n");
				return -1;
			}
			file = optarg;
			guid = &efi_guid_image_type_uboot_raw;
			break;
		case 'i':
			index = strtoul(optarg, NULL, 0);
			break;
		case 'I':
			instance = strtoul(optarg, NULL, 0);
			break;
		case 'K':
			if (pkey_file) {
				printf("Public Key already specified\n");
				return -1;
			}
			pkey_file = optarg;
			break;
		case 'D':
			if (dtb_file) {
				printf("DTB file already specified\n");
				return -1;
			}
			dtb_file = optarg;
			break;
#if IS_ENABLED(CONFIG_EFI_CAPSULE_AUTHENTICATE)
		case 'P':
			if (privkey_file) {
				printf("Private Key already specified\n");
				return -1;
			}
			privkey_file = optarg;
			break;
		case 'C':
			if (cert_file) {
				printf("Certificate file already specified\n");
				return -1;
			}
			cert_file = optarg;
			break;
		case 'm':
			mcount = strtoul(optarg, NULL, 0);
			break;
		case 'd':
			dump_sig = 1;
			break;
#endif
		case 'O':
			overlay = true;
			break;
		case 'h':
			print_usage();
			return 0;
		}
	}

	/* check necessary parameters */
	if ((file && (!(optind < argc) ||
		      (privkey_file && !cert_file) ||
		      (!privkey_file && cert_file))) ||
	    ((pkey_file && !dtb_file) ||
	     (!pkey_file && dtb_file))) {
		print_usage();
		exit(EXIT_FAILURE);
	}

	if (pkey_file && dtb_file) {
		ret = add_public_key(pkey_file, dtb_file, overlay);
		if (ret == -1) {
			printf("Adding public key to the dtb failed\n");
			exit(EXIT_FAILURE);
		}
	}

	if ((optind < argc) &&
	    create_fwbin(argv[optind], file, guid, index, instance,
			 mcount, privkey_file, cert_file)
			< 0) {
		printf("Creating firmware capsule failed\n");
		exit(EXIT_FAILURE);
	}

	exit(EXIT_SUCCESS);
}
