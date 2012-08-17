/*
 * Copyright 2011 Calxeda, Inc.
 *
 * See file CREDITS for list of people who contributed to this
 * project.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA 02111-1307 USA
 */

#include <linux/ctype.h>
#include "common.h"

/*
 * This is what a UUID string looks like.
 *
 * x is a hexadecimal character. fields are separated by '-'s. When converting
 * to a binary UUID, le means the field should be converted to little endian,
 * and be means it should be converted to big endian.
 *
 * 0        9    14   19   24
 * xxxxxxxx-xxxx-xxxx-xxxx-xxxxxxxxxxxx
 *    le     le   le   be       be
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

void uuid_str_to_bin(const char *uuid, unsigned char *out)
{
	uint16_t tmp16;
	uint32_t tmp32;
	uint64_t tmp64;

	if (!uuid || !out)
		return;

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
}
