// SPDX-License-Identifier: GPL-2.0+

#include <linux/xxhash.h>
#include <linux/unaligned/access_ok.h>
#include <linux/types.h>
#include <u-boot/sha256.h>
#include <u-boot/crc.h>

static u32 btrfs_crc32c_table[256];

void btrfs_hash_init(void)
{
	static int inited = 0;

	if (!inited) {
		crc32c_init(btrfs_crc32c_table, 0x82F63B78);
		inited = 1;
	}
}

int hash_sha256(const u8 *buf, size_t length, u8 *out)
{
	sha256_context ctx;

	sha256_starts(&ctx);
	sha256_update(&ctx, buf, length);
	sha256_finish(&ctx, out);

	return 0;
}

int hash_xxhash(const u8 *buf, size_t length, u8 *out)
{
	u64 hash;

	hash = xxh64(buf, length, 0);
	put_unaligned_le64(hash, out);

	return 0;
}

int hash_crc32c(const u8 *buf, size_t length, u8 *out)
{
	u32 crc;

	crc = crc32c_cal((u32)~0, (char *)buf, length, btrfs_crc32c_table);
	put_unaligned_le32(~crc, out);

	return 0;
}

u32 crc32c(u32 seed, const void * data, size_t len)
{
	return crc32c_cal(seed, data, len, btrfs_crc32c_table);
}
