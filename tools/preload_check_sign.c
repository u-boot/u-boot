// SPDX-License-Identifier: GPL-2.0+
/*
 * Check a file including a preload header including a signature
 *
 * Copyright (c) 2025 Paul HENRYS <paul.henrys_ext@softathome.com>
 *
 * Binman makes it possible to generate a preload header signing part or the
 * complete file. The tool preload_check_sign allows to verify and authenticate
 * a file starting with a preload header.
 */
#include <stdio.h>
#include <unistd.h>
#include <openssl/pem.h>
#include <openssl/evp.h>
#include <openssl/err.h>
#include <image.h>

extern void image_pre_load_sig_set_info(struct image_sig_info *info);
extern int image_pre_load_sig(ulong addr);

static void usage(char *cmdname)
{
	fprintf(stderr, "Usage: %s -f file -k PEM key file\n"
			"          -f ==> set file which should be checked\n"
			"          -k ==> PEM key file\n"
			"          -a ==> algo (default: sha256,rsa2048)\n"
			"          -p ==> padding (default: pkcs-1.5)\n"
			"          -h ==> help\n",
		cmdname);
	exit(EXIT_FAILURE);
}

int main(int argc, char **argv)
{
	int ret = 0;
	char cmdname[256];
	char *file = NULL;
	char *keyfile = NULL;
	int c;
	FILE *fp = NULL;
	FILE *fp_key = NULL;
	size_t bytes;
	long filesize;
	void *buffer = NULL;
	EVP_PKEY *pkey = NULL;
	char *algo = "sha256,rsa2048";
	char *padding = "pkcs-1.5";
	struct image_sig_info info = {0};

	strncpy(cmdname, *argv, sizeof(cmdname) - 1);
	cmdname[sizeof(cmdname) - 1] = '\0';
	while ((c = getopt(argc, argv, "f:k:a:p:h")) != -1)
		switch (c) {
		case 'f':
			file = optarg;
			break;
		case 'k':
			keyfile = optarg;
			break;
		case 'a':
			algo = optarg;
			break;
		case 'p':
			padding = optarg;
			break;
		default:
			usage(cmdname);
			break;
	}

	if (!file) {
		fprintf(stderr, "%s: Missing file\n", *argv);
		usage(*argv);
	}

	if (!keyfile) {
		fprintf(stderr, "%s: Missing key file\n", *argv);
		usage(*argv);
	}

	fp = fopen(file, "r");
	if (!fp) {
		fprintf(stderr, "Error opening file: %s\n", file);
		ret = EXIT_FAILURE;
		goto out;
	}

	fseek(fp, 0, SEEK_END);
	filesize = ftell(fp);
	rewind(fp);

	buffer = malloc(filesize);
	if (!buffer) {
		fprintf(stderr, "Memory allocation failed");
		ret = EXIT_FAILURE;
		goto out;
	}

	bytes = fread(buffer, 1, filesize, fp);
	if (bytes != filesize) {
		fprintf(stderr, "Error reading file\n");
		ret = EXIT_FAILURE;
		goto out;
	}

	fp_key = fopen(keyfile, "r");
	if (!fp_key) {
		fprintf(stderr, "Error opening file: %s\n", keyfile);
		ret = EXIT_FAILURE;
		goto out;
	}

	/* Attempt to read the private key */
	pkey = PEM_read_PrivateKey(fp_key, NULL, NULL, NULL);
	if (!pkey) {
		/* If private key reading fails, try reading as a public key */
		fseek(fp_key, 0, SEEK_SET);
		pkey = PEM_read_PUBKEY(fp_key, NULL, NULL, NULL);
	}
	if (!pkey) {
		fprintf(stderr, "Unable to retrieve the public key: %s\n",
			ERR_error_string(ERR_get_error(), NULL));
		ret = EXIT_FAILURE;
		goto out;
	}

	info.algo_name = algo;
	info.padding_name = padding;
	info.key = (uint8_t *)pkey;
	info.mandatory = 1;
	info.sig_size = EVP_PKEY_size(pkey);
	if (info.sig_size < 0) {
		fprintf(stderr, "Fail to retrieve the signature size: %s\n",
			ERR_error_string(ERR_get_error(), NULL));
		ret = EXIT_FAILURE;
		goto out;
	}

	/* Compute signature information */
	info.sig_info.name     = info.algo_name;
	info.sig_info.padding  = image_get_padding_algo(info.padding_name);
	info.sig_info.checksum = image_get_checksum_algo(info.sig_info.name);
	info.sig_info.crypto   = image_get_crypto_algo(info.sig_info.name);
	info.sig_info.key      = info.key;
	info.sig_info.keylen   = info.key_len;

	/* Check the signature */
	image_pre_load_sig_set_info(&info);
	ret = image_pre_load_sig((ulong)buffer);
out:
	if (fp)
		fclose(fp);
	if (fp_key)
		fclose(fp_key);
	if (info.key)
		EVP_PKEY_free(pkey);
	free(buffer);

	exit(ret);
}
