// SPDX-License-Identifier: GPL-2.0-or-later
// Copyright Contributors to the U-Boot project.

#include "imagetool.h"
#include <inttypes.h>
#include <u-boot/sha256.h>

/*
 * Image contain data in the following order:
 *  Nonce	16 byte
 *  Header	64 byte
 *  Digest	32 byte
 *  Padding	align up to 4K
 *  Payload
 */

#define HEADER_MAGIC		0x4c4d4140	/* @AML */
#define HEADER_OFFSET		0x10		/* 16 */
#define HEADER_SIZE		0x40		/* 64 */
#define PAYLOAD_OFFSET		0x1000		/* 4096 */

struct amlimage_header {
	uint32_t magic;
	uint32_t total_size;
	uint8_t header_size;
	uint8_t root_key_index;
	uint8_t version_major;
	uint8_t version_minor;
	uint32_t padding1;
	uint32_t digest_type;
	uint32_t digest_offset;
	uint32_t digest_size;
	uint32_t data_offset;
	uint32_t key_type;
	uint32_t key_offset;
	uint32_t key_size;
	uint32_t data_size;
	uint32_t payload_type;
	uint32_t payload_offset;
	uint32_t payload_size;
	uint32_t padding2;
} __packed;

struct amlimage_variant {
	const char *name;
	const struct amlimage_header hdr;
};

#define VARIANT(name, major, minor, size)	\
	{ name, { .magic = HEADER_MAGIC, .header_size = HEADER_SIZE, \
		  .version_major = major, .version_minor = minor, \
		  .payload_size = size, } }

static const struct amlimage_variant variants[] = {
	VARIANT("gxbb", 1, 0, 0xb000),
	VARIANT("gxl",  1, 1, 0xb000),
	VARIANT("gxm",  1, 1, 0xb000),
	VARIANT("axg",  1, 1, 0xb000),
	VARIANT("g12a", 1, 1, 0xf000),
	VARIANT("g12b", 1, 1, 0xf000),
	VARIANT("sm1",  1, 1, 0xf000),
};

static const struct amlimage_variant *amlimage_get_variant(const char *name)
{
	if (!name)
		return NULL;

	for (int i = 0; i < ARRAY_SIZE(variants); i++)
		if (!strcmp(name, variants[i].name))
			return &variants[i];

	return NULL;
}

static int amlimage_check_params(struct image_tool_params *params)
{
	const struct amlimage_variant *variant =
		amlimage_get_variant(params->imagename);
	int datafile_size;

	if (params->lflag || params->iflag)
		return EXIT_SUCCESS;

	if (!variant) {
		fprintf(stderr, "%s: unsupported image name: %s\n",
			params->cmdname, params->imagename);
		exit(EXIT_FAILURE);
	}

	datafile_size = imagetool_get_filesize(params, params->datafile);
	if (datafile_size < 0) {
		exit(EXIT_FAILURE);
	} else if (datafile_size > variant->hdr.payload_size) {
		fprintf(stderr, "%s: datafile is too large (%#x > %#x)\n",
			params->cmdname, datafile_size,
			variant->hdr.payload_size);
		exit(EXIT_FAILURE);
	}

	return EXIT_SUCCESS;
}

static int amlimage_verify_header(unsigned char *buf, int size,
				  struct image_tool_params *params)
{
	const struct amlimage_header *hdr = (void *)buf + HEADER_OFFSET;

	if (size >= HEADER_OFFSET + HEADER_SIZE + SHA256_SUM_LEN &&
	    hdr->magic == HEADER_MAGIC && hdr->header_size == HEADER_SIZE &&
	    hdr->version_major == 1 && hdr->version_minor <= 1)
		return 0;

	return -1;
}

static void amlimage_print_header(const void *buf,
				  struct image_tool_params *params)
{
	const struct amlimage_header *hdr = buf + HEADER_OFFSET;
	uint8_t digest[SHA256_SUM_LEN];
	sha256_context ctx;
	bool valid;

	printf("Amlogic Boot Image %" PRIu8 ".%" PRIu8 "\n",
	       hdr->version_major, hdr->version_minor);
	printf("Total size: %" PRIu32 "\n", hdr->total_size);
	printf("Digest %" PRIu32 ": %" PRIu32 " @ 0x%" PRIx32 "\n",
	       hdr->digest_type, hdr->digest_size, hdr->digest_offset);
	printf("Key %" PRIu32 ": %" PRIu32 " @ 0x%" PRIx32 "\n",
	       hdr->key_type, hdr->key_size, hdr->key_offset);
	printf("Payload %" PRIu32 ": %" PRIu32 " @ 0x%" PRIx32 "\n",
	       hdr->payload_type, hdr->payload_size, hdr->payload_offset);

	if (hdr->digest_type == 0) {
		/* sha256 digest (normal boot) */
		sha256_starts(&ctx);

		/* Header and data is used as input for sha256 digest */
		sha256_update(&ctx, (void *)hdr, hdr->header_size);
		sha256_update(&ctx, (void *)hdr + hdr->data_offset, hdr->data_size);
		sha256_finish(&ctx, digest);

		valid = !memcmp((void *)hdr + hdr->digest_offset,
				digest, SHA256_SUM_LEN);

		printf("Data: %" PRIu32 " @ 0x%" PRIx32 " - %s\n",
		       hdr->data_size, hdr->data_offset, valid ? "OK" : "BAD");
	} else {
		/* RSA (secure boot) */
		printf("Data: %" PRIu32 " @ 0x%" PRIx32 " - Secure Boot\n",
		       hdr->data_size, hdr->data_offset);
	}
}

static void amlimage_set_header(void *buf, struct stat *sbuf, int ifd,
				struct image_tool_params *params)
{
	struct amlimage_header *hdr = buf + HEADER_OFFSET;
	sha256_context ctx;

	/* Use header size as initial size */
	hdr->total_size = hdr->header_size;

	/* Use sha256 digest (normal boot) */
	hdr->digest_type = 0;
	/* The sha256 digest is stored directly following the header */
	hdr->digest_offset = hdr->total_size;
	/* Unknown if this is used as block size instead of digest size */
	hdr->digest_size = 512;
	hdr->total_size += hdr->digest_size;

	/* Use key as padding so that payload ends up 4K aligned in TZRAM */
	hdr->key_type = 0;
	hdr->key_offset = hdr->total_size;
	hdr->key_size = PAYLOAD_OFFSET - HEADER_OFFSET - hdr->key_offset;
	hdr->total_size += hdr->key_size;

	/* With padding above payload will have a 0x1000 offset in TZRAM */
	hdr->payload_type = 0;
	hdr->payload_offset = hdr->total_size;
	/* Payload size has already been copied from the variant header */
	hdr->total_size += hdr->payload_size;

	/* Set the data range to be used as input for sha256 digest */
	hdr->data_offset = hdr->digest_offset + SHA256_SUM_LEN;
	hdr->data_size = hdr->total_size - hdr->data_offset;

	sha256_starts(&ctx);
	/* Header and data is used as input for sha256 digest */
	sha256_update(&ctx, (void *)hdr, hdr->header_size);
	sha256_update(&ctx, (void *)hdr + hdr->data_offset, hdr->data_size);
	/* Write sha256 digest to the 32 bytes directly following the header */
	sha256_finish(&ctx, (void *)hdr + hdr->digest_offset);
}

static int amlimage_extract_subimage(void *buf,
				     struct image_tool_params *params)
{
	const struct amlimage_header *hdr = buf + HEADER_OFFSET;

	/* Save payload as the subimage */
	return imagetool_save_subimage(params->outfile,
				       (ulong)hdr + hdr->payload_offset,
				       hdr->payload_size);
}

static int amlimage_check_image_type(uint8_t type)
{
	if (type == IH_TYPE_AMLIMAGE)
		return EXIT_SUCCESS;

	return EXIT_FAILURE;
}

static int amlimage_vrec_header(struct image_tool_params *params,
				struct image_type_params *tparams)
{
	const struct amlimage_variant *variant =
		amlimage_get_variant(params->imagename);
	const struct amlimage_header *hdr = &variant->hdr;

	/* Use payload offset as header size, datafile will be appended */
	tparams->header_size = PAYLOAD_OFFSET;

	tparams->hdr = calloc(1, tparams->header_size);
	if (!tparams->hdr) {
		fprintf(stderr, "%s: Can't alloc header: %s\n",
			params->cmdname, strerror(errno));
		exit(EXIT_FAILURE);
	}

	/* Start with a copy of the variant header */
	memcpy(tparams->hdr + HEADER_OFFSET, hdr, hdr->header_size);

	/* Pad up to payload size of the variant header */
	return hdr->payload_size - params->file_size;
}

/*
 * amlimage parameters
 */
U_BOOT_IMAGE_TYPE(
	amlimage,
	"Amlogic Boot Image",
	0,
	NULL,
	amlimage_check_params,
	amlimage_verify_header,
	amlimage_print_header,
	amlimage_set_header,
	amlimage_extract_subimage,
	amlimage_check_image_type,
	NULL,
	amlimage_vrec_header
);
