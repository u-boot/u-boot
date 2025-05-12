// SPDX-License-Identifier: GPL-2.0+ OR BSD-3-Clause
/*
 * Copyright (C) 2018, STMicroelectronics - All Rights Reserved
 */

#include <image.h>
#include "imagetool.h"

/* magic ='S' 'T' 'M' 0x32 */
#define HEADER_MAGIC be32_to_cpu(0x53544D32)
#define VER_MAJOR	2
#define VER_MINOR	1
#define VER_VARIANT	0
#define HEADER_VERSION_V1	0x1
#define HEADER_VERSION_V2	0x2
/* default option : bit0 => no signature */
#define HEADER_DEFAULT_OPTION	(cpu_to_le32(0x00000001))
/* default binary type for U-Boot */
#define HEADER_TYPE_UBOOT	(cpu_to_le32(0x00000000))
#define PADDING_HEADER_MAGIC	(cpu_to_le32(0xFFFF5453))
#define PADDING_HEADER_FLAG	(1ULL << 31)
#define PADDING_HEADER_LENGTH	0x180

struct stm32_header_v1 {
	uint32_t magic_number;
	uint8_t image_signature[64];
	uint32_t image_checksum;
	uint8_t header_version[4];
	uint32_t image_length;
	uint32_t image_entry_point;
	uint32_t reserved1;
	uint32_t load_address;
	uint32_t reserved2;
	uint32_t version_number;
	/* V1.0 specific content */
	uint32_t option_flags;
	uint32_t ecdsa_algorithm;
	uint8_t ecdsa_public_key[64];
	uint8_t padding[83];
	uint8_t binary_type;
};

struct stm32_header_v2 {
	uint32_t magic_number;
	uint8_t image_signature[64];
	uint32_t image_checksum;
	uint8_t header_version[4];
	uint32_t image_length;
	uint32_t image_entry_point;
	uint32_t reserved1;
	uint32_t load_address;
	uint32_t reserved2;
	uint32_t version_number;
	/* V2.0 specific content */
	uint32_t extension_flags;
	uint32_t extension_headers_length;
	uint32_t binary_type;
	uint8_t padding[16];
	uint32_t extension_header_type;
	uint32_t extension_header_length;
	uint8_t extension_padding[376];
};

static struct stm32_header_v1 stm32image_header_v1;
static struct stm32_header_v2 stm32image_header_v2;

static uint32_t stm32image_checksum(void *start, uint32_t len,
				    uint32_t header_size)
{
	uint32_t csum = 0;
	uint8_t *p;

	if (len < header_size) {
		return 0;
	}

	p = (unsigned char *)start + header_size;
	len -= header_size;

	while (len > 0) {
		csum += *p;
		p++;
		len--;
	}

	return csum;
}

static int stm32image_check_image_types_v1(uint8_t type)
{
	if (type == IH_TYPE_STM32IMAGE)
		return EXIT_SUCCESS;
	return EXIT_FAILURE;
}

static int stm32image_check_image_types_v2(uint8_t type)
{
	if (type == IH_TYPE_STM32IMAGE_V2)
		return EXIT_SUCCESS;
	return EXIT_FAILURE;
}

static int stm32image_verify_header_v1(unsigned char *ptr, int image_size,
				       struct image_tool_params *params)
{
	struct stm32_header_v1 *stm32hdr = (struct stm32_header_v1 *)ptr;
	int i;

	if (image_size < sizeof(struct stm32_header_v1))
		return -1;
	if (stm32hdr->magic_number != HEADER_MAGIC)
		return -1;
	if (stm32hdr->header_version[VER_MAJOR] != HEADER_VERSION_V1)
		return -1;
	if (stm32hdr->reserved1 || stm32hdr->reserved2)
		return -1;
	for (i = 0; i < (sizeof(stm32hdr->padding) / 4); i++) {
		if (stm32hdr->padding[i] != 0)
			return -1;
	}

	return 0;
}

static int stm32image_verify_header_v2(unsigned char *ptr, int image_size,
				       struct image_tool_params *params)
{
	struct stm32_header_v2 *stm32hdr = (struct stm32_header_v2 *)ptr;
	int i;

	if (image_size < sizeof(struct stm32_header_v2))
		return -1;
	if (stm32hdr->magic_number != HEADER_MAGIC)
		return -1;
	if (stm32hdr->header_version[VER_MAJOR] != HEADER_VERSION_V2)
		return -1;
	if (stm32hdr->reserved1 || stm32hdr->reserved2)
		return -1;
	for (i = 0; i < (sizeof(stm32hdr->padding) / 4); i++) {
		if (stm32hdr->padding[i] != 0)
			return -1;
	}

	return 0;
}

static void stm32image_print_header(const void *ptr, struct image_tool_params *params)
{
	struct stm32_header_v1 *stm32hdr_v1 = (struct stm32_header_v1 *)ptr;
	struct stm32_header_v2 *stm32hdr_v2 = (struct stm32_header_v2 *)ptr;

	printf("Image Type   : STMicroelectronics STM32 V%d.%d\n",
	       stm32hdr_v1->header_version[VER_MAJOR],
	       stm32hdr_v1->header_version[VER_MINOR]);
	printf("Image Size   : %lu bytes\n",
	       (unsigned long)le32_to_cpu(stm32hdr_v1->image_length));
	printf("Image Load   : 0x%08x\n",
	       le32_to_cpu(stm32hdr_v1->load_address));
	printf("Entry Point  : 0x%08x\n",
	       le32_to_cpu(stm32hdr_v1->image_entry_point));
	printf("Checksum     : 0x%08x\n",
	       le32_to_cpu(stm32hdr_v1->image_checksum));
	switch (stm32hdr_v1->header_version[VER_MAJOR]) {
	case HEADER_VERSION_V1:
		printf("Option     : 0x%08x\n",
		       le32_to_cpu(stm32hdr_v1->option_flags));
		printf("BinaryType : 0x%08x\n",
		       le32_to_cpu(stm32hdr_v1->binary_type));
		break;

	case HEADER_VERSION_V2:
		printf("Extension    : 0x%08x\n",
		       le32_to_cpu(stm32hdr_v2->extension_flags));
		break;

	default:
		printf("Incorrect header version\n");
	}
}

static void stm32image_set_header_v1(void *ptr, struct stat *sbuf, int ifd,
				     struct image_tool_params *params)
{
	struct stm32_header_v1 *stm32hdr = (struct stm32_header_v1 *)ptr;

	stm32hdr->magic_number = HEADER_MAGIC;
	stm32hdr->version_number = cpu_to_le32(0);

	stm32hdr->header_version[VER_MAJOR] = HEADER_VERSION_V1;
	stm32hdr->option_flags = HEADER_DEFAULT_OPTION;
	stm32hdr->ecdsa_algorithm = cpu_to_le32(1);
	stm32hdr->binary_type = HEADER_TYPE_UBOOT;

	stm32hdr->load_address = cpu_to_le32(params->addr);
	stm32hdr->image_entry_point = cpu_to_le32(params->ep);
	stm32hdr->image_length = cpu_to_le32((uint32_t)sbuf->st_size -
					     sizeof(*stm32hdr));
	stm32hdr->image_checksum =
		cpu_to_le32(stm32image_checksum(ptr, sbuf->st_size,
						sizeof(*stm32hdr)));
}

static void stm32image_set_header_v2(void *ptr, struct stat *sbuf, int ifd,
				     struct image_tool_params *params)
{
	struct stm32_header_v2 *stm32hdr = (struct stm32_header_v2 *)ptr;

	stm32hdr->magic_number = HEADER_MAGIC;
	stm32hdr->version_number = cpu_to_le32(0);

	stm32hdr->header_version[VER_MAJOR] = HEADER_VERSION_V2;
	stm32hdr->extension_flags =
		cpu_to_le32(PADDING_HEADER_FLAG);
	stm32hdr->extension_headers_length =
		cpu_to_le32(PADDING_HEADER_LENGTH);
	stm32hdr->extension_header_type =
		cpu_to_le32(PADDING_HEADER_MAGIC);
	stm32hdr->extension_header_length =
		cpu_to_le32(PADDING_HEADER_LENGTH);

	stm32hdr->load_address = cpu_to_le32(params->addr);
	stm32hdr->image_entry_point = cpu_to_le32(params->ep);
	stm32hdr->image_length = cpu_to_le32((uint32_t)sbuf->st_size -
					     sizeof(*stm32hdr));
	stm32hdr->image_checksum =
		cpu_to_le32(stm32image_checksum(ptr, sbuf->st_size,
						sizeof(*stm32hdr)));
}

/*
 * stm32image parameters
 */
U_BOOT_IMAGE_TYPE(
	stm32image,
	"STMicroelectronics STM32MP Image support",
	sizeof(struct stm32_header_v1),
	(void *)&stm32image_header_v1,
	NULL,
	stm32image_verify_header_v1,
	stm32image_print_header,
	stm32image_set_header_v1,
	NULL,
	stm32image_check_image_types_v1,
	NULL,
	NULL
);

U_BOOT_IMAGE_TYPE(
	stm32imagev2,
	"STMicroelectronics STM32MP Image V2.0 support",
	sizeof(struct stm32_header_v2),
	(void *)&stm32image_header_v2,
	NULL,
	stm32image_verify_header_v2,
	stm32image_print_header,
	stm32image_set_header_v2,
	NULL,
	stm32image_check_image_types_v2,
	NULL,
	NULL
);
