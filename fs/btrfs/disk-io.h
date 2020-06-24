// SPDX-License-Identifier: GPL-2.0+
#ifndef __BTRFS_DISK_IO_H__
#define __BTRFS_DISK_IO_H__

#include "crypto/hash.h"
#include "ctree.h"
#include "disk-io.h"

static inline u64 btrfs_name_hash(const char *name, int len)
{
	u32 crc;

	crc = crc32c((u32)~1, (unsigned char *)name, len);

	return (u64)crc;
}

int btrfs_csum_data(u16 csum_type, const u8 *data, u8 *out, size_t len);

#endif
