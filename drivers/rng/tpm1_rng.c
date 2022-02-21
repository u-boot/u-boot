// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (c) 2022, Linaro Limited
 */

#include <common.h>
#include <dm.h>
#include <rng.h>
#include <tpm-utils.h>
#include <tpm-v1.h>

#define TPM_HEADER_SIZE		10

#define TPMV1_DATA_OFFSET	14

static int tpm1_rng_read(struct udevice *dev, void *data, size_t count)
{
	const u8 command[14] = {
		0x0, 0xc1,		/* TPM_TAG */
		0x0, 0x0, 0x0, 0xe,	/* parameter size */
		0x0, 0x0, 0x0, 0x46,	/* TPM_COMMAND_CODE */
	};
	const size_t length_offset = TPM_HEADER_SIZE;
	const size_t data_size_offset = TPM_HEADER_SIZE;
	const size_t data_offset = TPMV1_DATA_OFFSET;
	u8 buf[COMMAND_BUFFER_SIZE], response[COMMAND_BUFFER_SIZE];
	size_t response_length = sizeof(response);
	u32 data_size;
	u8 *out = data;

	while (count > 0) {
		u32 this_bytes = min(count,
				     sizeof(response) - data_offset);
		u32 err;

		if (pack_byte_string(buf, sizeof(buf), "sd",
				     0, command, sizeof(command),
				     length_offset, this_bytes))
			return TPM_LIB_ERROR;
		err = tpm_sendrecv_command(dev->parent, buf, response,
					   &response_length);
		if (err)
			return err;
		if (unpack_byte_string(response, response_length, "d",
				       data_size_offset, &data_size))
			return TPM_LIB_ERROR;
		if (data_size > count)
			return TPM_LIB_ERROR;
		if (unpack_byte_string(response, response_length, "s",
				       data_offset, out, data_size))
			return TPM_LIB_ERROR;

		count -= data_size;
		out += data_size;
	}

	return 0;
}

static const struct dm_rng_ops tpm1_rng_ops = {
	.read = tpm1_rng_read,
};

U_BOOT_DRIVER(tpm1_rng) = {
	.name		= "tpm1-rng",
	.id		= UCLASS_RNG,
	.ops		= &tpm1_rng_ops,
};
