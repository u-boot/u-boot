// SPDX-License-Identifier: GPL-2.0+
/*
 * Optional gzip compression for QR code payloads.
 */

#include <gzip.h>
#include <linux/string.h>
#include <linux/types.h>
#include <qrcodegen.h>

#define QRGZ_MAGIC_LEN	4

static const u8 qrgz_magic[QRGZ_MAGIC_LEN] = { 'Q', 'R', 'G', 'Z' };

bool qrcodegen_encodeBinaryCompressed(uint8_t dataAndTemp[], size_t dataLen,
				      uint8_t qrcode[], enum qrcodegen_Ecc ecl)
{
	size_t buf_max = qrcodegen_BUFFER_LEN_FOR_VERSION(qrcodegen_VERSION_MAX);
	unsigned long out_len;
	u8 *scratch;

	if (qrcodegen_encodeBinary(dataAndTemp, dataLen, qrcode, ecl,
				   qrcodegen_VERSION_MIN, qrcodegen_VERSION_MAX,
				   qrcodegen_Mask_AUTO, true))
		return true;

	if (buf_max < QRGZ_MAGIC_LEN)
		return false;

	scratch = qrcode;
	out_len = buf_max;
	if (gzip(scratch, &out_len, dataAndTemp, dataLen) != 0)
		return false;

	if (out_len + QRGZ_MAGIC_LEN > buf_max)
		return false;

	memmove(dataAndTemp + QRGZ_MAGIC_LEN, scratch, out_len);
	memcpy(dataAndTemp, qrgz_magic, QRGZ_MAGIC_LEN);

	return qrcodegen_encodeBinary(dataAndTemp, out_len + QRGZ_MAGIC_LEN,
				      qrcode, ecl, qrcodegen_VERSION_MIN,
				      qrcodegen_VERSION_MAX, qrcodegen_Mask_AUTO,
				      true);
}
