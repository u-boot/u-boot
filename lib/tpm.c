/*
 * Copyright (c) 2013 The Chromium OS Authors.
 * Coypright (c) 2013 Guntermann & Drunck GmbH
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <common.h>
#include <dm.h>
#include <tpm.h>
#include <asm/unaligned.h>
#include <u-boot/sha1.h>

/* Internal error of TPM command library */
#define TPM_LIB_ERROR	((uint32_t)~0u)

/* Useful constants */
enum {
	COMMAND_BUFFER_SIZE		= 256,
	TPM_REQUEST_HEADER_LENGTH	= 10,
	TPM_RESPONSE_HEADER_LENGTH	= 10,
	PCR_DIGEST_LENGTH		= 20,
	DIGEST_LENGTH			= 20,
	TPM_REQUEST_AUTH_LENGTH		= 45,
	TPM_RESPONSE_AUTH_LENGTH	= 41,
	/* some max lengths, valid for RSA keys <= 2048 bits */
	TPM_KEY12_MAX_LENGTH		= 618,
	TPM_PUBKEY_MAX_LENGTH		= 288,
};

#ifdef CONFIG_TPM_AUTH_SESSIONS

#ifndef CONFIG_SHA1
#error "TPM_AUTH_SESSIONS require SHA1 to be configured, too"
#endif /* !CONFIG_SHA1 */

struct session_data {
	int		valid;
	uint32_t	handle;
	uint8_t		nonce_even[DIGEST_LENGTH];
	uint8_t		nonce_odd[DIGEST_LENGTH];
};

static struct session_data oiap_session = {0, };

#endif /* CONFIG_TPM_AUTH_SESSIONS */

/**
 * Pack data into a byte string.  The data types are specified in
 * the format string: 'b' means unsigned byte, 'w' unsigned word,
 * 'd' unsigned double word, and 's' byte string.  The data are a
 * series of offsets and values (for type byte string there are also
 * lengths).  The data values are packed into the byte string
 * sequentially, and so a latter value could over-write a former
 * value.
 *
 * @param str		output string
 * @param size		size of output string
 * @param format	format string
 * @param ...		data points
 * @return 0 on success, non-0 on error
 */
int pack_byte_string(uint8_t *str, size_t size, const char *format, ...)
{
	va_list args;
	size_t offset = 0, length = 0;
	uint8_t *data = NULL;
	uint32_t value = 0;

	va_start(args, format);
	for (; *format; format++) {
		switch (*format) {
		case 'b':
			offset = va_arg(args, size_t);
			value = va_arg(args, int);
			length = 1;
			break;
		case 'w':
			offset = va_arg(args, size_t);
			value = va_arg(args, int);
			length = 2;
			break;
		case 'd':
			offset = va_arg(args, size_t);
			value = va_arg(args, uint32_t);
			length = 4;
			break;
		case 's':
			offset = va_arg(args, size_t);
			data = va_arg(args, uint8_t *);
			length = va_arg(args, uint32_t);
			break;
		default:
			debug("Couldn't recognize format string\n");
			return -1;
		}

		if (offset + length > size)
			return -1;

		switch (*format) {
		case 'b':
			str[offset] = value;
			break;
		case 'w':
			put_unaligned_be16(value, str + offset);
			break;
		case 'd':
			put_unaligned_be32(value, str + offset);
			break;
		case 's':
			memcpy(str + offset, data, length);
			break;
		}
	}
	va_end(args);

	return 0;
}

/**
 * Unpack data from a byte string.  The data types are specified in
 * the format string: 'b' means unsigned byte, 'w' unsigned word,
 * 'd' unsigned double word, and 's' byte string.  The data are a
 * series of offsets and pointers (for type byte string there are also
 * lengths).
 *
 * @param str		output string
 * @param size		size of output string
 * @param format	format string
 * @param ...		data points
 * @return 0 on success, non-0 on error
 */
int unpack_byte_string(const uint8_t *str, size_t size, const char *format, ...)
{
	va_list args;
	size_t offset = 0, length = 0;
	uint8_t *ptr8 = NULL;
	uint16_t *ptr16 = NULL;
	uint32_t *ptr32 = NULL;

	va_start(args, format);
	for (; *format; format++) {
		switch (*format) {
		case 'b':
			offset = va_arg(args, size_t);
			ptr8 = va_arg(args, uint8_t *);
			length = 1;
			break;
		case 'w':
			offset = va_arg(args, size_t);
			ptr16 = va_arg(args, uint16_t *);
			length = 2;
			break;
		case 'd':
			offset = va_arg(args, size_t);
			ptr32 = va_arg(args, uint32_t *);
			length = 4;
			break;
		case 's':
			offset = va_arg(args, size_t);
			ptr8 = va_arg(args, uint8_t *);
			length = va_arg(args, uint32_t);
			break;
		default:
			debug("Couldn't recognize format string\n");
			return -1;
		}

		if (offset + length > size)
			return -1;

		switch (*format) {
		case 'b':
			*ptr8 = str[offset];
			break;
		case 'w':
			*ptr16 = get_unaligned_be16(str + offset);
			break;
		case 'd':
			*ptr32 = get_unaligned_be32(str + offset);
			break;
		case 's':
			memcpy(ptr8, str + offset, length);
			break;
		}
	}
	va_end(args);

	return 0;
}

/**
 * Get TPM command size.
 *
 * @param command	byte string of TPM command
 * @return command size of the TPM command
 */
static uint32_t tpm_command_size(const void *command)
{
	const size_t command_size_offset = 2;
	return get_unaligned_be32(command + command_size_offset);
}

/**
 * Get TPM response return code, which is one of TPM_RESULT values.
 *
 * @param response	byte string of TPM response
 * @return return code of the TPM response
 */
static uint32_t tpm_return_code(const void *response)
{
	const size_t return_code_offset = 6;
	return get_unaligned_be32(response + return_code_offset);
}

/**
 * Send a TPM command and return response's return code, and optionally
 * return response to caller.
 *
 * @param command	byte string of TPM command
 * @param response	output buffer for TPM response, or NULL if the
 *			caller does not care about it
 * @param size_ptr	output buffer size (input parameter) and TPM
 *			response length (output parameter); this parameter
 *			is a bidirectional
 * @return return code of the TPM response
 */
static uint32_t tpm_sendrecv_command(const void *command,
		void *response, size_t *size_ptr)
{
	struct udevice *dev;
	int ret;
	uint8_t response_buffer[COMMAND_BUFFER_SIZE];
	size_t response_length;
	uint32_t err;

	if (response) {
		response_length = *size_ptr;
	} else {
		response = response_buffer;
		response_length = sizeof(response_buffer);
	}

	ret = uclass_first_device(UCLASS_TPM, &dev);
	if (ret)
		return ret;
	err = tpm_xfer(dev, command, tpm_command_size(command),
		       response, &response_length);

	if (err < 0)
		return TPM_LIB_ERROR;
	if (size_ptr)
		*size_ptr = response_length;

	return tpm_return_code(response);
}

int tpm_init(void)
{
	int err;
	struct udevice *dev;

	err = uclass_first_device(UCLASS_TPM, &dev);
	if (err || !dev)
		return err;
	return tpm_open(dev);
}

uint32_t tpm_startup(enum tpm_startup_type mode)
{
	const uint8_t command[12] = {
		0x0, 0xc1, 0x0, 0x0, 0x0, 0xc, 0x0, 0x0, 0x0, 0x99, 0x0, 0x0,
	};
	const size_t mode_offset = 10;
	uint8_t buf[COMMAND_BUFFER_SIZE];

	if (pack_byte_string(buf, sizeof(buf), "sw",
				0, command, sizeof(command),
				mode_offset, mode))
		return TPM_LIB_ERROR;

	return tpm_sendrecv_command(buf, NULL, NULL);
}

uint32_t tpm_self_test_full(void)
{
	const uint8_t command[10] = {
		0x0, 0xc1, 0x0, 0x0, 0x0, 0xa, 0x0, 0x0, 0x0, 0x50,
	};
	return tpm_sendrecv_command(command, NULL, NULL);
}

uint32_t tpm_continue_self_test(void)
{
	const uint8_t command[10] = {
		0x0, 0xc1, 0x0, 0x0, 0x0, 0xa, 0x0, 0x0, 0x0, 0x53,
	};
	return tpm_sendrecv_command(command, NULL, NULL);
}

uint32_t tpm_nv_define_space(uint32_t index, uint32_t perm, uint32_t size)
{
	const uint8_t command[101] = {
		0x0, 0xc1,		/* TPM_TAG */
		0x0, 0x0, 0x0, 0x65,	/* parameter size */
		0x0, 0x0, 0x0, 0xcc,	/* TPM_COMMAND_CODE */
		/* TPM_NV_DATA_PUBLIC->... */
		0x0, 0x18,		/* ...->TPM_STRUCTURE_TAG */
		0, 0, 0, 0,		/* ...->TPM_NV_INDEX */
		/* TPM_NV_DATA_PUBLIC->TPM_PCR_INFO_SHORT */
		0x0, 0x3,
		0, 0, 0,
		0x1f,
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
		/* TPM_NV_DATA_PUBLIC->TPM_PCR_INFO_SHORT */
		0x0, 0x3,
		0, 0, 0,
		0x1f,
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
		/* TPM_NV_ATTRIBUTES->... */
		0x0, 0x17,		/* ...->TPM_STRUCTURE_TAG */
		0, 0, 0, 0,		/* ...->attributes */
		/* End of TPM_NV_ATTRIBUTES */
		0,			/* bReadSTClear */
		0,			/* bWriteSTClear */
		0,			/* bWriteDefine */
		0, 0, 0, 0,		/* size */
	};
	const size_t index_offset = 12;
	const size_t perm_offset = 70;
	const size_t size_offset = 77;
	uint8_t buf[COMMAND_BUFFER_SIZE];

	if (pack_byte_string(buf, sizeof(buf), "sddd",
				0, command, sizeof(command),
				index_offset, index,
				perm_offset, perm,
				size_offset, size))
		return TPM_LIB_ERROR;

	return tpm_sendrecv_command(buf, NULL, NULL);
}

uint32_t tpm_nv_read_value(uint32_t index, void *data, uint32_t count)
{
	const uint8_t command[22] = {
		0x0, 0xc1, 0x0, 0x0, 0x0, 0x16, 0x0, 0x0, 0x0, 0xcf,
	};
	const size_t index_offset = 10;
	const size_t length_offset = 18;
	const size_t data_size_offset = 10;
	const size_t data_offset = 14;
	uint8_t buf[COMMAND_BUFFER_SIZE], response[COMMAND_BUFFER_SIZE];
	size_t response_length = sizeof(response);
	uint32_t data_size;
	uint32_t err;

	if (pack_byte_string(buf, sizeof(buf), "sdd",
				0, command, sizeof(command),
				index_offset, index,
				length_offset, count))
		return TPM_LIB_ERROR;
	err = tpm_sendrecv_command(buf, response, &response_length);
	if (err)
		return err;
	if (unpack_byte_string(response, response_length, "d",
				data_size_offset, &data_size))
		return TPM_LIB_ERROR;
	if (data_size > count)
		return TPM_LIB_ERROR;
	if (unpack_byte_string(response, response_length, "s",
				data_offset, data, data_size))
		return TPM_LIB_ERROR;

	return 0;
}

uint32_t tpm_nv_write_value(uint32_t index, const void *data, uint32_t length)
{
	const uint8_t command[256] = {
		0x0, 0xc1, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0xcd,
	};
	const size_t command_size_offset = 2;
	const size_t index_offset = 10;
	const size_t length_offset = 18;
	const size_t data_offset = 22;
	const size_t write_info_size = 12;
	const uint32_t total_length =
		TPM_REQUEST_HEADER_LENGTH + write_info_size + length;
	uint8_t buf[COMMAND_BUFFER_SIZE], response[COMMAND_BUFFER_SIZE];
	size_t response_length = sizeof(response);
	uint32_t err;

	if (pack_byte_string(buf, sizeof(buf), "sddds",
				0, command, sizeof(command),
				command_size_offset, total_length,
				index_offset, index,
				length_offset, length,
				data_offset, data, length))
		return TPM_LIB_ERROR;
	err = tpm_sendrecv_command(buf, response, &response_length);
	if (err)
		return err;

	return 0;
}

uint32_t tpm_extend(uint32_t index, const void *in_digest, void *out_digest)
{
	const uint8_t command[34] = {
		0x0, 0xc1, 0x0, 0x0, 0x0, 0x22, 0x0, 0x0, 0x0, 0x14,
	};
	const size_t index_offset = 10;
	const size_t in_digest_offset = 14;
	const size_t out_digest_offset = 10;
	uint8_t buf[COMMAND_BUFFER_SIZE];
	uint8_t response[TPM_RESPONSE_HEADER_LENGTH + PCR_DIGEST_LENGTH];
	size_t response_length = sizeof(response);
	uint32_t err;

	if (pack_byte_string(buf, sizeof(buf), "sds",
				0, command, sizeof(command),
				index_offset, index,
				in_digest_offset, in_digest,
				PCR_DIGEST_LENGTH))
		return TPM_LIB_ERROR;
	err = tpm_sendrecv_command(buf, response, &response_length);
	if (err)
		return err;

	if (unpack_byte_string(response, response_length, "s",
				out_digest_offset, out_digest,
				PCR_DIGEST_LENGTH))
		return TPM_LIB_ERROR;

	return 0;
}

uint32_t tpm_pcr_read(uint32_t index, void *data, size_t count)
{
	const uint8_t command[14] = {
		0x0, 0xc1, 0x0, 0x0, 0x0, 0xe, 0x0, 0x0, 0x0, 0x15,
	};
	const size_t index_offset = 10;
	const size_t out_digest_offset = 10;
	uint8_t buf[COMMAND_BUFFER_SIZE], response[COMMAND_BUFFER_SIZE];
	size_t response_length = sizeof(response);
	uint32_t err;

	if (count < PCR_DIGEST_LENGTH)
		return TPM_LIB_ERROR;

	if (pack_byte_string(buf, sizeof(buf), "sd",
				0, command, sizeof(command),
				index_offset, index))
		return TPM_LIB_ERROR;
	err = tpm_sendrecv_command(buf, response, &response_length);
	if (err)
		return err;
	if (unpack_byte_string(response, response_length, "s",
				out_digest_offset, data, PCR_DIGEST_LENGTH))
		return TPM_LIB_ERROR;

	return 0;
}

uint32_t tpm_tsc_physical_presence(uint16_t presence)
{
	const uint8_t command[12] = {
		0x0, 0xc1, 0x0, 0x0, 0x0, 0xc, 0x40, 0x0, 0x0, 0xa, 0x0, 0x0,
	};
	const size_t presence_offset = 10;
	uint8_t buf[COMMAND_BUFFER_SIZE];

	if (pack_byte_string(buf, sizeof(buf), "sw",
				0, command, sizeof(command),
				presence_offset, presence))
		return TPM_LIB_ERROR;

	return tpm_sendrecv_command(buf, NULL, NULL);
}

uint32_t tpm_read_pubek(void *data, size_t count)
{
	const uint8_t command[30] = {
		0x0, 0xc1, 0x0, 0x0, 0x0, 0x1e, 0x0, 0x0, 0x0, 0x7c,
	};
	const size_t response_size_offset = 2;
	const size_t data_offset = 10;
	const size_t header_and_checksum_size = TPM_RESPONSE_HEADER_LENGTH + 20;
	uint8_t response[COMMAND_BUFFER_SIZE + TPM_PUBEK_SIZE];
	size_t response_length = sizeof(response);
	uint32_t data_size;
	uint32_t err;

	err = tpm_sendrecv_command(command, response, &response_length);
	if (err)
		return err;
	if (unpack_byte_string(response, response_length, "d",
				response_size_offset, &data_size))
		return TPM_LIB_ERROR;
	if (data_size < header_and_checksum_size)
		return TPM_LIB_ERROR;
	data_size -= header_and_checksum_size;
	if (data_size > count)
		return TPM_LIB_ERROR;
	if (unpack_byte_string(response, response_length, "s",
				data_offset, data, data_size))
		return TPM_LIB_ERROR;

	return 0;
}

uint32_t tpm_force_clear(void)
{
	const uint8_t command[10] = {
		0x0, 0xc1, 0x0, 0x0, 0x0, 0xa, 0x0, 0x0, 0x0, 0x5d,
	};

	return tpm_sendrecv_command(command, NULL, NULL);
}

uint32_t tpm_physical_enable(void)
{
	const uint8_t command[10] = {
		0x0, 0xc1, 0x0, 0x0, 0x0, 0xa, 0x0, 0x0, 0x0, 0x6f,
	};

	return tpm_sendrecv_command(command, NULL, NULL);
}

uint32_t tpm_physical_disable(void)
{
	const uint8_t command[10] = {
		0x0, 0xc1, 0x0, 0x0, 0x0, 0xa, 0x0, 0x0, 0x0, 0x70,
	};

	return tpm_sendrecv_command(command, NULL, NULL);
}

uint32_t tpm_physical_set_deactivated(uint8_t state)
{
	const uint8_t command[11] = {
		0x0, 0xc1, 0x0, 0x0, 0x0, 0xb, 0x0, 0x0, 0x0, 0x72,
	};
	const size_t state_offset = 10;
	uint8_t buf[COMMAND_BUFFER_SIZE];

	if (pack_byte_string(buf, sizeof(buf), "sb",
				0, command, sizeof(command),
				state_offset, state))
		return TPM_LIB_ERROR;

	return tpm_sendrecv_command(buf, NULL, NULL);
}

uint32_t tpm_get_capability(uint32_t cap_area, uint32_t sub_cap,
		void *cap, size_t count)
{
	const uint8_t command[22] = {
		0x0, 0xc1,		/* TPM_TAG */
		0x0, 0x0, 0x0, 0x16,	/* parameter size */
		0x0, 0x0, 0x0, 0x65,	/* TPM_COMMAND_CODE */
		0x0, 0x0, 0x0, 0x0,	/* TPM_CAPABILITY_AREA */
		0x0, 0x0, 0x0, 0x4,	/* subcap size */
		0x0, 0x0, 0x0, 0x0,	/* subcap value */
	};
	const size_t cap_area_offset = 10;
	const size_t sub_cap_offset = 18;
	const size_t cap_offset = 14;
	const size_t cap_size_offset = 10;
	uint8_t buf[COMMAND_BUFFER_SIZE], response[COMMAND_BUFFER_SIZE];
	size_t response_length = sizeof(response);
	uint32_t cap_size;
	uint32_t err;

	if (pack_byte_string(buf, sizeof(buf), "sdd",
				0, command, sizeof(command),
				cap_area_offset, cap_area,
				sub_cap_offset, sub_cap))
		return TPM_LIB_ERROR;
	err = tpm_sendrecv_command(buf, response, &response_length);
	if (err)
		return err;
	if (unpack_byte_string(response, response_length, "d",
				cap_size_offset, &cap_size))
		return TPM_LIB_ERROR;
	if (cap_size > response_length || cap_size > count)
		return TPM_LIB_ERROR;
	if (unpack_byte_string(response, response_length, "s",
				cap_offset, cap, cap_size))
		return TPM_LIB_ERROR;

	return 0;
}

uint32_t tpm_get_permanent_flags(struct tpm_permanent_flags *pflags)
{
	const uint8_t command[22] = {
		0x0, 0xc1,		/* TPM_TAG */
		0x0, 0x0, 0x0, 0x16,	/* parameter size */
		0x0, 0x0, 0x0, 0x65,	/* TPM_COMMAND_CODE */
		0x0, 0x0, 0x0, 0x4,	/* TPM_CAP_FLAG_PERM */
		0x0, 0x0, 0x0, 0x4,	/* subcap size */
		0x0, 0x0, 0x1, 0x8,	/* subcap value */
	};
	uint8_t response[COMMAND_BUFFER_SIZE];
	size_t response_length = sizeof(response);
	uint32_t err;

	err = tpm_sendrecv_command(command, response, &response_length);
	if (err)
		return err;
	memcpy(pflags, response + TPM_HEADER_SIZE, sizeof(*pflags));

	return 0;
}

uint32_t tpm_get_permissions(uint32_t index, uint32_t *perm)
{
	const uint8_t command[22] = {
		0x0, 0xc1,		/* TPM_TAG */
		0x0, 0x0, 0x0, 0x16,	/* parameter size */
		0x0, 0x0, 0x0, 0x65,	/* TPM_COMMAND_CODE */
		0x0, 0x0, 0x0, 0x11,
		0x0, 0x0, 0x0, 0x4,
	};
	const size_t index_offset = 18;
	const size_t perm_offset = 60;
	uint8_t buf[COMMAND_BUFFER_SIZE], response[COMMAND_BUFFER_SIZE];
	size_t response_length = sizeof(response);
	uint32_t err;

	if (pack_byte_string(buf, sizeof(buf), "d", 0, command, sizeof(command),
			     index_offset, index))
		return TPM_LIB_ERROR;
	err = tpm_sendrecv_command(buf, response, &response_length);
	if (err)
		return err;
	if (unpack_byte_string(response, response_length, "d",
			       perm_offset, perm))
		return TPM_LIB_ERROR;

	return 0;
}

#ifdef CONFIG_TPM_AUTH_SESSIONS

/**
 * Fill an authentication block in a request.
 * This func can create the first as well as the second auth block (for
 * double authorized commands).
 *
 * @param request	pointer to the request (w/ uninitialised auth data)
 * @param request_len0	length of the request without auth data
 * @param handles_len	length of the handles area in request
 * @param auth_session	pointer to the (valid) auth session to be used
 * @param request_auth	pointer to the auth block of the request to be filled
 * @param auth		authentication data (HMAC key)
 */
static uint32_t create_request_auth(const void *request, size_t request_len0,
	size_t handles_len,
	struct session_data *auth_session,
	void *request_auth, const void *auth)
{
	uint8_t hmac_data[DIGEST_LENGTH * 3 + 1];
	sha1_context hash_ctx;
	const size_t command_code_offset = 6;
	const size_t auth_nonce_odd_offset = 4;
	const size_t auth_continue_offset = 24;
	const size_t auth_auth_offset = 25;

	if (!auth_session || !auth_session->valid)
		return TPM_LIB_ERROR;

	sha1_starts(&hash_ctx);
	sha1_update(&hash_ctx, request + command_code_offset, 4);
	if (request_len0 > TPM_REQUEST_HEADER_LENGTH + handles_len)
		sha1_update(&hash_ctx,
			    request + TPM_REQUEST_HEADER_LENGTH + handles_len,
			    request_len0 - TPM_REQUEST_HEADER_LENGTH
			    - handles_len);
	sha1_finish(&hash_ctx, hmac_data);

	sha1_starts(&hash_ctx);
	sha1_update(&hash_ctx, auth_session->nonce_odd, DIGEST_LENGTH);
	sha1_update(&hash_ctx, hmac_data, sizeof(hmac_data));
	sha1_finish(&hash_ctx, auth_session->nonce_odd);

	if (pack_byte_string(request_auth, TPM_REQUEST_AUTH_LENGTH, "dsb",
			     0, auth_session->handle,
			     auth_nonce_odd_offset, auth_session->nonce_odd,
			     DIGEST_LENGTH,
			     auth_continue_offset, 1))
		return TPM_LIB_ERROR;
	if (pack_byte_string(hmac_data, sizeof(hmac_data), "ss",
			     DIGEST_LENGTH,
			     auth_session->nonce_even,
			     DIGEST_LENGTH,
			     2 * DIGEST_LENGTH,
			     request_auth + auth_nonce_odd_offset,
			     DIGEST_LENGTH + 1))
		return TPM_LIB_ERROR;
	sha1_hmac(auth, DIGEST_LENGTH, hmac_data, sizeof(hmac_data),
		  request_auth + auth_auth_offset);

	return TPM_SUCCESS;
}

/**
 * Verify an authentication block in a response.
 * Since this func updates the nonce_even in the session data it has to be
 * called when receiving a succesfull AUTH response.
 * This func can verify the first as well as the second auth block (for
 * double authorized commands).
 *
 * @param command_code	command code of the request
 * @param response	pointer to the request (w/ uninitialised auth data)
 * @param handles_len	length of the handles area in response
 * @param auth_session	pointer to the (valid) auth session to be used
 * @param response_auth	pointer to the auth block of the response to be verified
 * @param auth		authentication data (HMAC key)
 */
static uint32_t verify_response_auth(uint32_t command_code,
	const void *response, size_t response_len0,
	size_t handles_len,
	struct session_data *auth_session,
	const void *response_auth, const void *auth)
{
	uint8_t hmac_data[DIGEST_LENGTH * 3 + 1];
	uint8_t computed_auth[DIGEST_LENGTH];
	sha1_context hash_ctx;
	const size_t return_code_offset = 6;
	const size_t auth_continue_offset = 20;
	const size_t auth_auth_offset = 21;
	uint8_t auth_continue;

	if (!auth_session || !auth_session->valid)
		return TPM_AUTHFAIL;
	if (pack_byte_string(hmac_data, sizeof(hmac_data), "d",
			     0, command_code))
		return TPM_LIB_ERROR;
	if (response_len0 < TPM_RESPONSE_HEADER_LENGTH)
		return TPM_LIB_ERROR;

	sha1_starts(&hash_ctx);
	sha1_update(&hash_ctx, response + return_code_offset, 4);
	sha1_update(&hash_ctx, hmac_data, 4);
	if (response_len0 > TPM_RESPONSE_HEADER_LENGTH + handles_len)
		sha1_update(&hash_ctx,
			    response + TPM_RESPONSE_HEADER_LENGTH + handles_len,
			    response_len0 - TPM_RESPONSE_HEADER_LENGTH
			    - handles_len);
	sha1_finish(&hash_ctx, hmac_data);

	memcpy(auth_session->nonce_even, response_auth, DIGEST_LENGTH);
	auth_continue = ((uint8_t *)response_auth)[auth_continue_offset];
	if (pack_byte_string(hmac_data, sizeof(hmac_data), "ssb",
			     DIGEST_LENGTH,
			     response_auth,
			     DIGEST_LENGTH,
			     2 * DIGEST_LENGTH,
			     auth_session->nonce_odd,
			     DIGEST_LENGTH,
			     3 * DIGEST_LENGTH,
			     auth_continue))
		return TPM_LIB_ERROR;

	sha1_hmac(auth, DIGEST_LENGTH, hmac_data, sizeof(hmac_data),
		  computed_auth);

	if (memcmp(computed_auth, response_auth + auth_auth_offset,
		   DIGEST_LENGTH))
		return TPM_AUTHFAIL;

	return TPM_SUCCESS;
}


uint32_t tpm_terminate_auth_session(uint32_t auth_handle)
{
	const uint8_t command[18] = {
		0x00, 0xc1,		/* TPM_TAG */
		0x00, 0x00, 0x00, 0x00,	/* parameter size */
		0x00, 0x00, 0x00, 0xba,	/* TPM_COMMAND_CODE */
		0x00, 0x00, 0x00, 0x00,	/* TPM_HANDLE */
		0x00, 0x00, 0x00, 0x02,	/* TPM_RESSOURCE_TYPE */
	};
	const size_t req_handle_offset = TPM_REQUEST_HEADER_LENGTH;
	uint8_t request[COMMAND_BUFFER_SIZE];

	if (pack_byte_string(request, sizeof(request), "sd",
			     0, command, sizeof(command),
			     req_handle_offset, auth_handle))
		return TPM_LIB_ERROR;
	if (oiap_session.valid && oiap_session.handle == auth_handle)
		oiap_session.valid = 0;

	return tpm_sendrecv_command(request, NULL, NULL);
}

uint32_t tpm_end_oiap(void)
{
	uint32_t err = TPM_SUCCESS;
	if (oiap_session.valid)
		err = tpm_terminate_auth_session(oiap_session.handle);
	return err;
}

uint32_t tpm_oiap(uint32_t *auth_handle)
{
	const uint8_t command[10] = {
		0x00, 0xc1,		/* TPM_TAG */
		0x00, 0x00, 0x00, 0x0a,	/* parameter size */
		0x00, 0x00, 0x00, 0x0a,	/* TPM_COMMAND_CODE */
	};
	const size_t res_auth_handle_offset = TPM_RESPONSE_HEADER_LENGTH;
	const size_t res_nonce_even_offset = TPM_RESPONSE_HEADER_LENGTH + 4;
	uint8_t response[COMMAND_BUFFER_SIZE];
	size_t response_length = sizeof(response);
	uint32_t err;

	if (oiap_session.valid)
		tpm_terminate_auth_session(oiap_session.handle);

	err = tpm_sendrecv_command(command, response, &response_length);
	if (err)
		return err;
	if (unpack_byte_string(response, response_length, "ds",
			       res_auth_handle_offset, &oiap_session.handle,
			       res_nonce_even_offset, &oiap_session.nonce_even,
			       (uint32_t)DIGEST_LENGTH))
		return TPM_LIB_ERROR;
	oiap_session.valid = 1;
	if (auth_handle)
		*auth_handle = oiap_session.handle;
	return 0;
}

uint32_t tpm_load_key2_oiap(uint32_t parent_handle,
		const void *key, size_t key_length,
		const void *parent_key_usage_auth,
		uint32_t *key_handle)
{
	const uint8_t command[14] = {
		0x00, 0xc2,		/* TPM_TAG */
		0x00, 0x00, 0x00, 0x00,	/* parameter size */
		0x00, 0x00, 0x00, 0x41,	/* TPM_COMMAND_CODE */
		0x00, 0x00, 0x00, 0x00,	/* parent handle */
	};
	const size_t req_size_offset = 2;
	const size_t req_parent_handle_offset = TPM_REQUEST_HEADER_LENGTH;
	const size_t req_key_offset = TPM_REQUEST_HEADER_LENGTH + 4;
	const size_t res_handle_offset = TPM_RESPONSE_HEADER_LENGTH;
	uint8_t request[sizeof(command) + TPM_KEY12_MAX_LENGTH
			+ TPM_REQUEST_AUTH_LENGTH];
	uint8_t response[COMMAND_BUFFER_SIZE];
	size_t response_length = sizeof(response);
	uint32_t err;

	if (!oiap_session.valid) {
		err = tpm_oiap(NULL);
		if (err)
			return err;
	}
	if (pack_byte_string(request, sizeof(request), "sdds",
			     0, command, sizeof(command),
			     req_size_offset,
			     sizeof(command) + key_length
			     + TPM_REQUEST_AUTH_LENGTH,
			     req_parent_handle_offset, parent_handle,
			     req_key_offset, key, key_length
		))
		return TPM_LIB_ERROR;

	err = create_request_auth(request, sizeof(command) + key_length, 4,
				&oiap_session,
				request + sizeof(command) + key_length,
				parent_key_usage_auth);
	if (err)
		return err;
	err = tpm_sendrecv_command(request, response, &response_length);
	if (err) {
		if (err == TPM_AUTHFAIL)
			oiap_session.valid = 0;
		return err;
	}

	err = verify_response_auth(0x00000041, response,
			response_length - TPM_RESPONSE_AUTH_LENGTH,
			4, &oiap_session,
			response + response_length - TPM_RESPONSE_AUTH_LENGTH,
			parent_key_usage_auth);
	if (err)
		return err;

	if (key_handle) {
		if (unpack_byte_string(response, response_length, "d",
				       res_handle_offset, key_handle))
			return TPM_LIB_ERROR;
	}

	return 0;
}

uint32_t tpm_get_pub_key_oiap(uint32_t key_handle, const void *usage_auth,
			void *pubkey, size_t *pubkey_len)
{
	const uint8_t command[14] = {
		0x00, 0xc2,		/* TPM_TAG */
		0x00, 0x00, 0x00, 0x00,	/* parameter size */
		0x00, 0x00, 0x00, 0x21,	/* TPM_COMMAND_CODE */
		0x00, 0x00, 0x00, 0x00,	/* key handle */
	};
	const size_t req_size_offset = 2;
	const size_t req_key_handle_offset = TPM_REQUEST_HEADER_LENGTH;
	const size_t res_pubkey_offset = TPM_RESPONSE_HEADER_LENGTH;
	uint8_t request[sizeof(command) + TPM_REQUEST_AUTH_LENGTH];
	uint8_t response[TPM_RESPONSE_HEADER_LENGTH + TPM_PUBKEY_MAX_LENGTH
			+ TPM_RESPONSE_AUTH_LENGTH];
	size_t response_length = sizeof(response);
	uint32_t err;

	if (!oiap_session.valid) {
		err = tpm_oiap(NULL);
		if (err)
			return err;
	}
	if (pack_byte_string(request, sizeof(request), "sdd",
			     0, command, sizeof(command),
			     req_size_offset,
			     (uint32_t)(sizeof(command)
			     + TPM_REQUEST_AUTH_LENGTH),
			     req_key_handle_offset, key_handle
		))
		return TPM_LIB_ERROR;
	err = create_request_auth(request, sizeof(command), 4, &oiap_session,
			request + sizeof(command), usage_auth);
	if (err)
		return err;
	err = tpm_sendrecv_command(request, response, &response_length);
	if (err) {
		if (err == TPM_AUTHFAIL)
			oiap_session.valid = 0;
		return err;
	}
	err = verify_response_auth(0x00000021, response,
			response_length - TPM_RESPONSE_AUTH_LENGTH,
			0, &oiap_session,
			response + response_length - TPM_RESPONSE_AUTH_LENGTH,
			usage_auth);
	if (err)
		return err;

	if (pubkey) {
		if ((response_length - TPM_RESPONSE_HEADER_LENGTH
			- TPM_RESPONSE_AUTH_LENGTH) > *pubkey_len)
			return TPM_LIB_ERROR;
		*pubkey_len = response_length - TPM_RESPONSE_HEADER_LENGTH
			- TPM_RESPONSE_AUTH_LENGTH;
		memcpy(pubkey, response + res_pubkey_offset,
		       response_length - TPM_RESPONSE_HEADER_LENGTH
		       - TPM_RESPONSE_AUTH_LENGTH);
	}

	return 0;
}

#endif /* CONFIG_TPM_AUTH_SESSIONS */
