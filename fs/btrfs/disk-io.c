// SPDX-License-Identifier: GPL-2.0+
#include <common.h>
#include <fs_internal.h>
#include <uuid.h>
#include <memalign.h>
#include "kernel-shared/btrfs_tree.h"
#include "disk-io.h"
#include "ctree.h"
#include "btrfs.h"
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

/*
 * Check if the super is valid:
 * - nodesize/sectorsize - minimum, maximum, alignment
 * - tree block starts   - alignment
 * - number of devices   - something sane
 * - sys array size      - maximum
 */
static int btrfs_check_super(struct btrfs_super_block *sb)
{
	u8 result[BTRFS_CSUM_SIZE];
	u16 csum_type;
	int csum_size;
	u8 *metadata_uuid;

	if (btrfs_super_magic(sb) != BTRFS_MAGIC)
		return -EIO;

	csum_type = btrfs_super_csum_type(sb);
	if (csum_type >= btrfs_super_num_csums()) {
		error("unsupported checksum algorithm %u", csum_type);
		return -EIO;
	}
	csum_size = btrfs_super_csum_size(sb);

	btrfs_csum_data(csum_type, (u8 *)sb + BTRFS_CSUM_SIZE,
			result, BTRFS_SUPER_INFO_SIZE - BTRFS_CSUM_SIZE);

	if (memcmp(result, sb->csum, csum_size)) {
		error("superblock checksum mismatch");
		return -EIO;
	}
	if (btrfs_super_root_level(sb) >= BTRFS_MAX_LEVEL) {
		error("tree_root level too big: %d >= %d",
			btrfs_super_root_level(sb), BTRFS_MAX_LEVEL);
		goto error_out;
	}
	if (btrfs_super_chunk_root_level(sb) >= BTRFS_MAX_LEVEL) {
		error("chunk_root level too big: %d >= %d",
			btrfs_super_chunk_root_level(sb), BTRFS_MAX_LEVEL);
		goto error_out;
	}
	if (btrfs_super_log_root_level(sb) >= BTRFS_MAX_LEVEL) {
		error("log_root level too big: %d >= %d",
			btrfs_super_log_root_level(sb), BTRFS_MAX_LEVEL);
		goto error_out;
	}

	if (!IS_ALIGNED(btrfs_super_root(sb), 4096)) {
		error("tree_root block unaligned: %llu", btrfs_super_root(sb));
		goto error_out;
	}
	if (!IS_ALIGNED(btrfs_super_chunk_root(sb), 4096)) {
		error("chunk_root block unaligned: %llu",
			btrfs_super_chunk_root(sb));
		goto error_out;
	}
	if (!IS_ALIGNED(btrfs_super_log_root(sb), 4096)) {
		error("log_root block unaligned: %llu",
			btrfs_super_log_root(sb));
		goto error_out;
	}
	if (btrfs_super_nodesize(sb) < 4096) {
		error("nodesize too small: %u < 4096",
			btrfs_super_nodesize(sb));
		goto error_out;
	}
	if (!IS_ALIGNED(btrfs_super_nodesize(sb), 4096)) {
		error("nodesize unaligned: %u", btrfs_super_nodesize(sb));
		goto error_out;
	}
	if (btrfs_super_sectorsize(sb) < 4096) {
		error("sectorsize too small: %u < 4096",
			btrfs_super_sectorsize(sb));
		goto error_out;
	}
	if (!IS_ALIGNED(btrfs_super_sectorsize(sb), 4096)) {
		error("sectorsize unaligned: %u", btrfs_super_sectorsize(sb));
		goto error_out;
	}
	if (btrfs_super_total_bytes(sb) == 0) {
		error("invalid total_bytes 0");
		goto error_out;
	}
	if (btrfs_super_bytes_used(sb) < 6 * btrfs_super_nodesize(sb)) {
		error("invalid bytes_used %llu", btrfs_super_bytes_used(sb));
		goto error_out;
	}
	if ((btrfs_super_stripesize(sb) != 4096)
		&& (btrfs_super_stripesize(sb) != btrfs_super_sectorsize(sb))) {
		error("invalid stripesize %u", btrfs_super_stripesize(sb));
		goto error_out;
	}

	if (btrfs_super_incompat_flags(sb) & BTRFS_FEATURE_INCOMPAT_METADATA_UUID)
		metadata_uuid = sb->metadata_uuid;
	else
		metadata_uuid = sb->fsid;

	if (memcmp(metadata_uuid, sb->dev_item.fsid, BTRFS_FSID_SIZE) != 0) {
		char fsid[BTRFS_UUID_UNPARSED_SIZE];
		char dev_fsid[BTRFS_UUID_UNPARSED_SIZE];

		uuid_unparse(sb->metadata_uuid, fsid);
		uuid_unparse(sb->dev_item.fsid, dev_fsid);
		error("dev_item UUID does not match fsid: %s != %s",
			dev_fsid, fsid);
		goto error_out;
	}

	/*
	 * Hint to catch really bogus numbers, bitflips or so
	 */
	if (btrfs_super_num_devices(sb) > (1UL << 31)) {
		error("suspicious number of devices: %llu",
			btrfs_super_num_devices(sb));
	}

	if (btrfs_super_num_devices(sb) == 0) {
		error("number of devices is 0");
		goto error_out;
	}

	/*
	 * Obvious sys_chunk_array corruptions, it must hold at least one key
	 * and one chunk
	 */
	if (btrfs_super_sys_array_size(sb) > BTRFS_SYSTEM_CHUNK_ARRAY_SIZE) {
		error("system chunk array too big %u > %u",
		      btrfs_super_sys_array_size(sb),
		      BTRFS_SYSTEM_CHUNK_ARRAY_SIZE);
		goto error_out;
	}
	if (btrfs_super_sys_array_size(sb) < sizeof(struct btrfs_disk_key)
			+ sizeof(struct btrfs_chunk)) {
		error("system chunk array too small %u < %zu",
		      btrfs_super_sys_array_size(sb),
		      sizeof(struct btrfs_disk_key) +
		      sizeof(struct btrfs_chunk));
		goto error_out;
	}

	return 0;

error_out:
	error("superblock checksum matches but it has invalid members");
	return -EIO;
}

/*
 * btrfs_read_dev_super - read a valid primary superblock from a block device
 * @desc,@part:	file descriptor of the device
 * @sb:		buffer where the superblock is going to be read in
 *
 * Unlike the btrfs-progs/kernel version, here we ony care about the first
 * super block, thus it's much simpler.
 */
int btrfs_read_dev_super(struct blk_desc *desc, struct disk_partition *part,
			 struct btrfs_super_block *sb)
{
	char tmp[BTRFS_SUPER_INFO_SIZE];
	struct btrfs_super_block *buf = (struct btrfs_super_block *)tmp;
	int ret;

	ret = __btrfs_devread(desc, part, tmp, BTRFS_SUPER_INFO_SIZE,
			      BTRFS_SUPER_INFO_OFFSET);
	if (ret < BTRFS_SUPER_INFO_SIZE)
		return -EIO;

	if (btrfs_super_bytenr(buf) != BTRFS_SUPER_INFO_OFFSET)
		return -EIO;

	if (btrfs_check_super(buf))
		return -EIO;

	memcpy(sb, buf, BTRFS_SUPER_INFO_SIZE);
	return 0;
}

int btrfs_read_superblock(void)
{
	ALLOC_CACHE_ALIGN_BUFFER(char, raw_sb, BTRFS_SUPER_INFO_SIZE);
	struct btrfs_super_block *sb = (struct btrfs_super_block *) raw_sb;
	int ret;


	btrfs_info.sb.generation = 0;

	ret = btrfs_read_dev_super(btrfs_blk_desc, btrfs_part_info, sb);
	if (ret < 0) {
		pr_debug("%s: No valid BTRFS superblock found!\n", __func__);
		return ret;
	}
	btrfs_super_block_to_cpu(sb);
	memcpy(&btrfs_info.sb, sb, sizeof(*sb));

	if (btrfs_info.sb.num_devices != 1) {
		printf("%s: Unsupported number of devices (%lli). This driver "
		       "only supports filesystem on one device.\n", __func__,
		       btrfs_info.sb.num_devices);
		return -1;
	}

	pr_debug("Chosen superblock with generation = %llu\n",
	      btrfs_info.sb.generation);

	return 0;
}
