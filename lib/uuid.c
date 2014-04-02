/*
 * Copyright 2011 Calxeda, Inc.
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <linux/ctype.h>
#include <errno.h>
#include <common.h>

#define UUID_STR_LEN		36

/*
 * UUID - Universally Unique IDentifier - 128 bits unique number.
 *        There are 5 versions and one variant of UUID defined by RFC4122
 *        specification. Depends on version uuid number base on a time,
 *        host name, MAC address or random data.
 *
 * UUID binary format (16 bytes):
 *
 * 4B-2B-2B-2B-6B (big endian - network byte order)
 *
 * UUID string is 36 length of characters (36 bytes):
 *
 * 0        9    14   19   24
 * xxxxxxxx-xxxx-xxxx-xxxx-xxxxxxxxxxxx
 *    be     be   be   be       be
 *
 * where x is a hexadecimal character. Fields are separated by '-'s.
 * When converting to a binary UUID, le means the field should be converted
 * to little endian and be means it should be converted to big endian.
 *
 * UUID is also used as GUID (Globally Unique Identifier) with the same binary
 * format but it differs in string format like below.
 *
 * GUID:
 * 0        9    14   19   24
 * xxxxxxxx-xxxx-xxxx-xxxx-xxxxxxxxxxxx
 *    le     le   le   be       be
 *
 * GUID is used e.g. in GPT (GUID Partition Table) as a partiions unique id.
 */

int uuid_str_valid(const char *uuid)
{
	int i, valid;

	if (uuid == NULL)
		return 0;

	for (i = 0, valid = 1; uuid[i] && valid; i++) {
		switch (i) {
		case 8: case 13: case 18: case 23:
			valid = (uuid[i] == '-');
			break;
		default:
			valid = isxdigit(uuid[i]);
			break;
		}
	}

	if (i != 36 || !valid)
		return 0;

	return 1;
}

int uuid_str_to_bin(char *uuid, unsigned char *out)
{
	uint16_t tmp16;
	uint32_t tmp32;
	uint64_t tmp64;

	if (!uuid || !out)
		return -EINVAL;

	if (strlen(uuid) != UUID_STR_LEN)
		return -EINVAL;

	tmp32 = cpu_to_le32(simple_strtoul(uuid, NULL, 16));
	memcpy(out, &tmp32, 4);

	tmp16 = cpu_to_le16(simple_strtoul(uuid + 9, NULL, 16));
	memcpy(out + 4, &tmp16, 2);

	tmp16 = cpu_to_le16(simple_strtoul(uuid + 14, NULL, 16));
	memcpy(out + 6, &tmp16, 2);

	tmp16 = cpu_to_be16(simple_strtoul(uuid + 19, NULL, 16));
	memcpy(out + 8, &tmp16, 2);

	tmp64 = cpu_to_be64(simple_strtoull(uuid + 24, NULL, 16));
	memcpy(out + 10, (char *)&tmp64 + 2, 6);

	return 0;
}

void uuid_bin_to_str(unsigned char *uuid, char *str)
{
	static const u8 le[16] = {3, 2, 1, 0, 5, 4, 7, 6, 8, 9, 10, 11,
				  12, 13, 14, 15};
	int i;

	for (i = 0; i < 16; i++) {
		sprintf(str, "%02x", uuid[le[i]]);
		str += 2;
		switch (i) {
		case 3:
		case 5:
		case 7:
		case 9:
			*str++ = '-';
			break;
		}
	}
}
