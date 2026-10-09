// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (C) 2026 Junhui Liu <junhui.liu@pigmoral.tech>
 *
 * The SpacemiT boot image consists of a fixed image header followed by an
 * aligned SPL payload and a trailing authentication area. The image header
 * contains two metadata headers and areas reserved for authentication data.
 *
 * The image layout is:
 *
 *   0x000             root key
 *   0x100             header0
 *   0x120             key metadata
 *   0x300             OEM keys
 *   0xb00             signature0
 *   0xfe0             header1
 *   0x1000            SPL payload, with its end aligned to 32 bytes
 *   payload end       signature1 (0x100 bytes)
 */

#include <image.h>
#include "imagetool.h"
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <u-boot/crc.h>

#define SMT_MAGIC		"AIHD"
#define SMT_AUTH_SIZE		0x100

#define SMT_FLAG_CRC32		(1U << 0)

/**
 * struct smtimage_header - metadata header within a SpacemiT boot image header
 *
 * @magic:	Magic (must be SMT_MAGIC)
 * @version:	Image version used when anti-rollback is enabled by eFuse
 * @secure:	Version slot used when anti-rollback is enabled by eFuse
 * @reserved:	Reserved
 * @image_size:	Header0 stores the image header size
 *		Header1 stores the aligned payload size
 * @load_addr:	Unused
 * @header_crc:	Header0 and Header1 store the CRC32 of the preceding 24 bytes on K3
 * @image_crc:	Header1 stores the CRC32 of the aligned payload on K3
 */
struct smtimage_header {
	uint8_t magic[4];
	uint8_t version;
	uint8_t secure;
	uint16_t reserved;
	uint64_t image_size;
	uint64_t load_addr;
	uint32_t header_crc;
	uint32_t image_crc;
} __packed;

/**
 * struct smtimage_image_header - fixed header area of a SpacemiT boot image
 *
 * @root_key:	Root public key
 * @header0:	Image layout and load information
 * @key_data:	Key metadata
 * @oem_keys:	OEM public-key area
 * @signature0:	Signature of the image configuration
 * @reserved:	Reserved
 * @header1:	SPL payload information
 */
struct smtimage_image_header {
	uint8_t root_key[0x100];
	struct smtimage_header header0;
	uint8_t key_data[0x1e0];
	uint8_t oem_keys[0x800];
	uint8_t signature0[SMT_AUTH_SIZE];
	uint8_t reserved[0x3e0];
	struct smtimage_header header1;
} __packed;

/**
 * struct smtimage_variant - SoC-specific image properties
 *
 * @name:	SoC name passed to mkimage with -n
 * @flags:	SoC-specific image flags
 */
struct smtimage_variant {
	const char *name;
	unsigned int flags;
};

static const struct smtimage_variant smtimage_variants[] = {
	{
		.name = "k3",
		.flags = SMT_FLAG_CRC32,
	},
};

static const struct smtimage_variant *smtimage_get_variant(const char *name)
{
	int i;

	if (!name)
		return NULL;

	for (i = 0; i < ARRAY_SIZE(smtimage_variants); i++)
		if (!strcmp(name, smtimage_variants[i].name))
			return &smtimage_variants[i];

	return NULL;
}

static void smtimage_init_header(struct smtimage_header *header, uint64_t image_size,
				 const struct smtimage_variant *variant)
{
	uint32_t crc;

	memcpy(header->magic, SMT_MAGIC, sizeof(header->magic));

	header->image_size = cpu_to_le64(image_size);

	if (variant->flags & SMT_FLAG_CRC32) {
		crc = crc32(0, (uint8_t *)header,
			    offsetof(struct smtimage_header, header_crc));
		header->header_crc = cpu_to_le32(crc);
	}
}

static int smtimage_check_params(struct image_tool_params *params)
{
	if (!smtimage_get_variant(params->imagename)) {
		fprintf(stderr, "%s: SpacemiT SoC must be specified with -n\n",
			params->cmdname);
		return EXIT_FAILURE;
	}

	if (!params->dflag)
		return EXIT_FAILURE;

	return EXIT_SUCCESS;
}

static void smtimage_print_header(const void *buf, struct image_tool_params *params)
{
	const struct smtimage_image_header *image_header = buf;
	const struct smtimage_header *header1 = &image_header->header1;
	uint64_t payload_size = le64_to_cpu(header1->image_size);

	printf("Image Type:   SpacemiT Boot Image\n");
	printf("SoC Variant:  %s\n", params->imagename);
	printf("Total Size:   %llu bytes\n",
	       (unsigned long long)(sizeof(*image_header) + payload_size + SMT_AUTH_SIZE));
	printf("Payload Size: %llu bytes\n", (unsigned long long)payload_size);
}

static void smtimage_set_header(void *buf, struct stat *sbuf, int infd,
				struct image_tool_params *params)
{
	const struct smtimage_variant *variant = smtimage_get_variant(params->imagename);
	struct smtimage_image_header *image_header = buf;
	struct smtimage_header *header0 = &image_header->header0;
	struct smtimage_header *header1 = &image_header->header1;
	size_t payload_size;
	uint32_t crc;

	payload_size = sbuf->st_size - sizeof(*image_header) - SMT_AUTH_SIZE;

	smtimage_init_header(header0, sizeof(*image_header), variant);
	smtimage_init_header(header1, payload_size, variant);

	if (variant->flags & SMT_FLAG_CRC32) {
		crc = crc32(0, (uint8_t *)(image_header + 1), payload_size);
		header1->image_crc = cpu_to_le32(crc);
	}
}

static int smtimage_check_image_type(uint8_t type)
{
	if (type == IH_TYPE_SMTIMAGE)
		return EXIT_SUCCESS;

	return EXIT_FAILURE;
}

static int smtimage_vrec_header(struct image_tool_params *params,
				struct image_type_params *tparams)
{
	size_t payload_size = params->file_size - tparams->header_size;

	tparams->hdr = calloc(tparams->header_size, 1);

	return ALIGN(payload_size, 32) - payload_size + SMT_AUTH_SIZE;
}

U_BOOT_IMAGE_TYPE(
	smtimage,
	"SpacemiT Boot Image support",
	sizeof(struct smtimage_image_header),
	NULL,
	smtimage_check_params,
	NULL,
	smtimage_print_header,
	smtimage_set_header,
	NULL,
	smtimage_check_image_type,
	NULL,
	smtimage_vrec_header
);
