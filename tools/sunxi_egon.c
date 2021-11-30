// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2018 Arm Ltd.
 */

#include "imagetool.h"
#include <image.h>

#include <sunxi_image.h>

/*
 * NAND requires 8K padding. SD/eMMC gets away with 512 bytes,
 * but let's use the larger padding to cover both.
 */
#define PAD_SIZE			8192

static int egon_check_params(struct image_tool_params *params)
{
	/* We just need a binary image file. */
	return !params->dflag;
}

static int egon_verify_header(unsigned char *ptr, int image_size,
			      struct image_tool_params *params)
{
	const struct boot_file_head *header = (void *)ptr;
	uint32_t length;

	/* First 4 bytes must be an ARM branch instruction. */
	if ((le32_to_cpu(header->b_instruction) & 0xff000000) != 0xea000000)
		return EXIT_FAILURE;

	if (memcmp(header->magic, BOOT0_MAGIC, sizeof(header->magic)))
		return EXIT_FAILURE;

	length = le32_to_cpu(header->length);
	/* Must be at least 512 byte aligned. */
	if (length & 511)
		return EXIT_FAILURE;

	/*
	 * Image could also contain U-Boot proper, so could be bigger.
	 * But it must not be shorter.
	 */
	if (image_size < length)
		return EXIT_FAILURE;

	return EXIT_SUCCESS;
}

static void egon_print_header(const void *buf)
{
	const struct boot_file_head *header = buf;

	printf("Allwinner eGON image, size: %d bytes\n",
	       le32_to_cpu(header->length));

	if (memcmp(header->spl_signature, SPL_SIGNATURE, 3))
		return;

	printf("\tSPL header version %d.%d\n",
	       header->spl_signature[3] >> SPL_MINOR_BITS,
	       header->spl_signature[3] & ((1U << SPL_MINOR_BITS) - 1));
	if (header->spl_signature[3] >= SPL_DT_HEADER_VERSION) {
		uint32_t dt_name_offs = le32_to_cpu(header->dt_name_offset);

		if (dt_name_offs > 0)
			printf("\tDT name: %s\n", (char *)buf + dt_name_offs);
	}
}

static void egon_set_header(void *buf, struct stat *sbuf, int infd,
			    struct image_tool_params *params)
{
	struct boot_file_head *header = buf;
	uint32_t *buf32 = buf;
	uint32_t checksum = 0, value;
	int i;

	/* Generate an ARM branch instruction to jump over the header. */
	value = 0xea000000 | (sizeof(struct boot_file_head) / 4 - 2);
	header->b_instruction = cpu_to_le32(value);

	memcpy(header->magic, BOOT0_MAGIC, sizeof(header->magic));
	header->check_sum = cpu_to_le32(BROM_STAMP_VALUE);
	header->length = cpu_to_le32(params->file_size);

	memcpy(header->spl_signature, SPL_SIGNATURE, 3);
	header->spl_signature[3] = SPL_ENV_HEADER_VERSION;

	/* If an image name has been provided, use it as the DT name. */
	if (params->imagename && params->imagename[0]) {
		if (strlen(params->imagename) > sizeof(header->string_pool) - 1)
			printf("WARNING: DT name too long for SPL header!\n");
		else {
			strcpy((char *)header->string_pool, params->imagename);
			value = offsetof(struct boot_file_head, string_pool);
			header->dt_name_offset = cpu_to_le32(value);
			header->spl_signature[3] = SPL_DT_HEADER_VERSION;
		}
	}

	/* Calculate the checksum. Yes, it's that simple. */
	for (i = 0; i < sbuf->st_size / 4; i++)
		checksum += le32_to_cpu(buf32[i]);
	header->check_sum = cpu_to_le32(checksum);
}

static int egon_check_image_type(uint8_t type)
{
	return type == IH_TYPE_SUNXI_EGON ? 0 : 1;
}

static int egon_vrec_header(struct image_tool_params *params,
			    struct image_type_params *tparams)
{
	tparams->hdr = calloc(sizeof(struct boot_file_head), 1);

	/* Return padding to 8K blocks. */
	return ALIGN(params->file_size, PAD_SIZE) - params->file_size;
}

U_BOOT_IMAGE_TYPE(
	sunxi_egon,
	"Allwinner eGON Boot Image support",
	sizeof(struct boot_file_head),
	NULL,
	egon_check_params,
	egon_verify_header,
	egon_print_header,
	egon_set_header,
	NULL,
	egon_check_image_type,
	NULL,
	egon_vrec_header
);
