// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (c) 2022, Linaro Limited
 */

#include <common.h>
#include <dm.h>
#include <rng.h>
#include <tpm-utils.h>
#include <tpm-v2.h>

#define TPM_HEADER_SIZE		10

#define TPMV2_DATA_OFFSET	12

static int tpm2_rng_read(struct udevice *dev, void *data, size_t count)
{
	const u8 command_v2[10] = {
		tpm_u16(TPM2_ST_NO_SESSIONS),
		tpm_u32(12),
		tpm_u32(TPM2_CC_GET_RANDOM),
	};
	u8 buf[COMMAND_BUFFER_SIZE], response[COMMAND_BUFFER_SIZE];

	const size_t data_size_offset = TPM_HEADER_SIZE;
	const size_t data_offset = TPMV2_DATA_OFFSET;
	size_t response_length = sizeof(response);
	u32 data_size;
	u8 *out = data;

	while (count > 0) {
		u32 this_bytes = min((size_t)count,
				     sizeof(response) - data_offset);
		u32 err;

		if (pack_byte_string(buf, sizeof(buf), "sw",
				     0, command_v2, sizeof(command_v2),
				     sizeof(command_v2), this_bytes))
			return TPM_LIB_ERROR;
		err = tpm_sendrecv_command(dev->parent, buf, response,
					   &response_length);
		if (err)
			return err;
		if (unpack_byte_string(response, response_length, "w",
				       data_size_offset, &data_size))
			return TPM_LIB_ERROR;
		if (data_size > this_bytes)
			return TPM_LIB_ERROR;
		if (unpack_byte_string(response, response_length, "s",
				       data_offset, out, data_size))
			return TPM_LIB_ERROR;

		count -= data_size;
		out += data_size;
	}

	return 0;
}

static const struct dm_rng_ops tpm2_rng_ops = {
	.read = tpm2_rng_read,
};

U_BOOT_DRIVER(tpm2_rng) = {
	.name		= "tpm2-rng",
	.id		= UCLASS_RNG,
	.ops		= &tpm2_rng_ops,
};
