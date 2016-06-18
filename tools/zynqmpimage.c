/*
 * Copyright (C) 2016 Michal Simek <michals@xilinx.com>
 * Copyright (C) 2015 Nathan Rossi <nathan@nathanrossi.com>
 *
 * SPDX-License-Identifier:	GPL-2.0+
 *
 * The following Boot Header format/structures and values are defined in the
 * following documents:
 *   * ug1085 ZynqMP TRM (Chapter 9, Table 9-3)
 *
 * Expected Header Size = 0x9C0
 * Forced as 'little' endian, 32-bit words
 *
 *  0x  0 - Interrupt table (8 words)
 *  ...     (Default value = 0xeafffffe)
 *  0x 1f
 *  0x 20 - Width detection
 *         * DEFAULT_WIDTHDETECTION    0xaa995566
 *  0x 24 - Image identifier
 *         * DEFAULT_IMAGEIDENTIFIER   0x584c4e58
 *  0x 28 - Encryption
 *         * 0x00000000 - None
 *         * 0xa5c3c5a3 - eFuse
 *         * 0xa5c3c5a7 - obfuscated key in eFUSE
 *         * 0x3a5c3c5a - bbRam
 *         * 0xa35c7ca5 - obfuscated key in boot header
 *  0x 2C - Image load
 *  0x 30 - Image offset
 *  0x 34 - PFW image length
 *  0x 38 - Total PFW image length
 *  0x 3C - Image length
 *  0x 40 - Total image length
 *  0x 44 - Image attributes
 *  0x 48 - Header checksum
 *  0x 4c - Obfuscated key
 *  ...
 *  0x 68
 *  0x 6c - Reserved
 *  0x 70 - User defined
 *  ...
 *  0x 9c
 *  0x a0 - Secure header initialization vector
 *  ...
 *  0x a8
 *  0x ac - Obfuscated key initialization vector
 *  ...
 *  0x b4
 *  0x b8 - Register Initialization, 511 Address and Data word pairs
 *         * List is terminated with an address of 0xffffffff or
 *  ...    * at the max number of entries
 *  0x8b4
 *  0x8b8 - Reserved
 *  ...
 *  0x9bf
 *  0x9c0 - Data/Image starts here or above
 */

#include "imagetool.h"
#include "mkimage.h"
#include <image.h>

#define HEADER_INTERRUPT_DEFAULT (cpu_to_le32(0xeafffffe))
#define HEADER_REGINIT_NULL (cpu_to_le32(0xffffffff))
#define HEADER_WIDTHDETECTION (cpu_to_le32(0xaa995566))
#define HEADER_IMAGEIDENTIFIER (cpu_to_le32(0x584c4e58))

enum {
	ENCRYPTION_EFUSE = 0xa5c3c5a3,
	ENCRYPTION_OEFUSE = 0xa5c3c5a7,
	ENCRYPTION_BBRAM = 0x3a5c3c5a,
	ENCRYPTION_OBBRAM = 0xa35c7ca5,
	ENCRYPTION_NONE = 0x0,
};

struct zynqmp_reginit {
	uint32_t address;
	uint32_t data;
};

#define HEADER_INTERRUPT_VECTORS	8
#define HEADER_REGINITS			256

struct zynqmp_header {
	uint32_t interrupt_vectors[HEADER_INTERRUPT_VECTORS]; /* 0x0 */
	uint32_t width_detection; /* 0x20 */
	uint32_t image_identifier; /* 0x24 */
	uint32_t encryption; /* 0x28 */
	uint32_t image_load; /* 0x2c */
	uint32_t image_offset; /* 0x30 */
	uint32_t pfw_image_length; /* 0x34 */
	uint32_t total_pfw_image_length; /* 0x38 */
	uint32_t image_size; /* 0x3c */
	uint32_t image_stored_size; /* 0x40 */
	uint32_t image_attributes; /* 0x44 */
	uint32_t checksum; /* 0x48 */
	uint32_t __reserved1[27]; /* 0x4c */
	struct zynqmp_reginit register_init[HEADER_REGINITS]; /* 0xb8 */
	uint32_t __reserved4[66]; /* 0x9c0 */
};

static struct zynqmp_header zynqmpimage_header;

static uint32_t zynqmpimage_checksum(struct zynqmp_header *ptr)
{
	uint32_t checksum = 0;

	if (ptr == NULL)
		return 0;

	checksum += le32_to_cpu(ptr->width_detection);
	checksum += le32_to_cpu(ptr->image_identifier);
	checksum += le32_to_cpu(ptr->encryption);
	checksum += le32_to_cpu(ptr->image_load);
	checksum += le32_to_cpu(ptr->image_offset);
	checksum += le32_to_cpu(ptr->pfw_image_length);
	checksum += le32_to_cpu(ptr->total_pfw_image_length);
	checksum += le32_to_cpu(ptr->image_size);
	checksum += le32_to_cpu(ptr->image_stored_size);
	checksum += le32_to_cpu(ptr->image_attributes);
	checksum = ~checksum;

	return cpu_to_le32(checksum);
}

static void zynqmpimage_default_header(struct zynqmp_header *ptr)
{
	int i;

	if (ptr == NULL)
		return;

	ptr->width_detection = HEADER_WIDTHDETECTION;
	ptr->image_attributes = 0x800;
	ptr->image_identifier = HEADER_IMAGEIDENTIFIER;
	ptr->encryption = cpu_to_le32(ENCRYPTION_NONE);

	/* Setup not-supported/constant/reserved fields */
	for (i = 0; i < HEADER_INTERRUPT_VECTORS; i++)
		ptr->interrupt_vectors[i] = HEADER_INTERRUPT_DEFAULT;

	for (i = 0; i < HEADER_REGINITS; i++) {
		ptr->register_init[i].address = HEADER_REGINIT_NULL;
		ptr->register_init[i].data = 0;
	}

	/*
	 * Certain reserved fields are required to be set to 0, ensure they are
	 * set as such.
	 */
	ptr->pfw_image_length = 0x0;
	ptr->total_pfw_image_length = 0x0;
}

/* mkimage glue functions */
static int zynqmpimage_verify_header(unsigned char *ptr, int image_size,
		struct image_tool_params *params)
{
	struct zynqmp_header *zynqhdr = (struct zynqmp_header *)ptr;

	if (image_size < sizeof(struct zynqmp_header))
		return -1;

	if (zynqhdr->width_detection != HEADER_WIDTHDETECTION)
		return -1;
	if (zynqhdr->image_identifier != HEADER_IMAGEIDENTIFIER)
		return -1;

	if (zynqmpimage_checksum(zynqhdr) != zynqhdr->checksum)
		return -1;

	return 0;
}

static void zynqmpimage_print_header(const void *ptr)
{
	struct zynqmp_header *zynqhdr = (struct zynqmp_header *)ptr;
	int i;

	printf("Image Type   : Xilinx Zynq Boot Image support\n");
	printf("Image Offset : 0x%08x\n", le32_to_cpu(zynqhdr->image_offset));
	printf("Image Size   : %lu bytes (%lu bytes packed)\n",
	       (unsigned long)le32_to_cpu(zynqhdr->image_size),
	       (unsigned long)le32_to_cpu(zynqhdr->image_stored_size));
	printf("Image Load   : 0x%08x\n", le32_to_cpu(zynqhdr->image_load));
	printf("Checksum     : 0x%08x\n", le32_to_cpu(zynqhdr->checksum));

	for (i = 0; i < HEADER_INTERRUPT_VECTORS; i++) {
		if (zynqhdr->interrupt_vectors[i] == HEADER_INTERRUPT_DEFAULT)
			continue;

		printf("Modified Interrupt Vector Address [%d]: 0x%08x\n", i,
		       le32_to_cpu(zynqhdr->interrupt_vectors[i]));
	}

	for (i = 0; i < HEADER_REGINITS; i++) {
		if (zynqhdr->register_init[i].address == HEADER_REGINIT_NULL)
			break;

		if (i == 0)
			printf("Custom Register Initialization:\n");

		printf("    @ 0x%08x -> 0x%08x\n",
		       le32_to_cpu(zynqhdr->register_init[i].address),
		       le32_to_cpu(zynqhdr->register_init[i].data));
	}
}

static int zynqmpimage_check_params(struct image_tool_params *params)
{
	if (!params)
		return 0;

	if (params->addr != 0x0) {
		fprintf(stderr, "Error: Load Address cannot be specified.\n");
		return -1;
	}

	/*
	 * If the entry point is specified ensure it is 64 byte aligned.
	 */
	if (params->eflag && (params->ep % 64 != 0)) {
		fprintf(stderr,
			"Error: Entry Point must be aligned to a 64-byte boundary.\n");
		return -1;
	}

	return !(params->lflag || params->dflag);
}

static int zynqmpimage_check_image_types(uint8_t type)
{
	if (type == IH_TYPE_ZYNQMPIMAGE)
		return EXIT_SUCCESS;
	return EXIT_FAILURE;
}

static void zynqmpimage_set_header(void *ptr, struct stat *sbuf, int ifd,
		struct image_tool_params *params)
{
	struct zynqmp_header *zynqhdr = (struct zynqmp_header *)ptr;
	zynqmpimage_default_header(zynqhdr);

	/* place image directly after header */
	zynqhdr->image_offset =
		cpu_to_le32((uint32_t)sizeof(struct zynqmp_header));
	zynqhdr->image_size = cpu_to_le32(params->file_size -
					  sizeof(struct zynqmp_header));
	zynqhdr->image_stored_size = zynqhdr->image_size;
	zynqhdr->image_load = 0xfffc0000;
	if (params->eflag)
		zynqhdr->image_load = cpu_to_le32((uint32_t)params->ep);

	zynqhdr->checksum = zynqmpimage_checksum(zynqhdr);
}

U_BOOT_IMAGE_TYPE(
	zynqmpimage,
	"Xilinx ZynqMP Boot Image support",
	sizeof(struct zynqmp_header),
	(void *)&zynqmpimage_header,
	zynqmpimage_check_params,
	zynqmpimage_verify_header,
	zynqmpimage_print_header,
	zynqmpimage_set_header,
	NULL,
	zynqmpimage_check_image_types,
	NULL,
	NULL
);
