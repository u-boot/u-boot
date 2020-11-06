// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2020, Bachmann electronic GmbH
 */

#include <common.h>
#include <smbios.h>

static inline int verify_checksum(const struct smbios_entry *e)
{
	/*
	 * Checksums for SMBIOS tables are calculated to have a value, so that
	 * the sum over all bytes yields zero (using unsigned 8 bit arithmetic).
	 */
	u8 *byte = (u8 *)e;
	u8 sum = 0;

	for (int i = 0; i < e->length; i++)
		sum += byte[i];

	return sum;
}

const struct smbios_entry *smbios_entry(u64 address, u32 size)
{
	const struct smbios_entry *entry = (struct smbios_entry *)(uintptr_t)address;

	if (!address | !size)
		return NULL;

	if (memcmp(entry->anchor, "_SM_", 4))
		return NULL;

	if (verify_checksum(entry))
		return NULL;

	return entry;
}

static const struct smbios_header *next_header(const struct smbios_header *curr)
{
	u8 *pos = ((u8 *)curr) + curr->length;

	/* search for _double_ NULL bytes */
	while (!((*pos == 0) && (*(pos + 1) == 0)))
		pos++;

	/* step behind the double NULL bytes */
	pos += 2;

	return (struct smbios_header *)pos;
}

const struct smbios_header *smbios_header(const struct smbios_entry *entry, int type)
{
	const unsigned int num_header = entry->struct_count;
	const struct smbios_header *header = (struct smbios_header *)entry->struct_table_address;

	for (unsigned int i = 0; i < num_header; i++) {
		if (header->type == type)
			return header;

		header = next_header(header);
	}

	return NULL;
}

static const char *string_from_smbios_table(const struct smbios_header *header,
                                           int idx)
{
	unsigned int i = 1;
	u8 *pos;

	if (!header)
		return NULL;

	pos = ((u8 *)header) + header->length;

	while (i < idx) {
		if (*pos == 0x0)
			i++;

		pos++;
	}

	return (const char *)pos;
}

const char *smbios_string(const struct smbios_header *header, int index)
{
	if (!header)
		return NULL;

	return string_from_smbios_table(header, index);
}
