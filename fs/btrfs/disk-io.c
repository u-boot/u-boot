// SPDX-License-Identifier: GPL-2.0+
#include <common.h>
#include <fs_internal.h>
#include "disk-io.h"
#include "crypto/hash.h"

int btrfs_csum_data(u16 csum_type, const u8 *data, u8 *out, size_t len)
{
	memset(out, 0, BTRFS_CSUM_SIZE);

	switch (csum_type) {
	case BTRFS_CSUM_TYPE_CRC32:
		return hash_crc32c(data, len, out);
	case BTRFS_CSUM_TYPE_XXHASH:
		return hash_xxhash(data, len, out);
	case BTRFS_CSUM_TYPE_SHA256:
		return hash_sha256(data, len, out);
	default:
		printf("Unknown csum type %d\n", csum_type);
		return -EINVAL;
	}
}
