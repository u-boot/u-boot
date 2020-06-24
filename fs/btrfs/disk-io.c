// SPDX-License-Identifier: GPL-2.0+
#include <common.h>
#include <fs_internal.h>
#include <uuid.h>
#include <memalign.h>
#include "kernel-shared/btrfs_tree.h"
#include "disk-io.h"
#include "ctree.h"
#include "btrfs.h"
#include "volumes.h"
#include "extent-io.h"
#include "crypto/hash.h"

/* specified errno for check_tree_block */
#define BTRFS_BAD_BYTENR		(-1)
#define BTRFS_BAD_FSID			(-2)
#define BTRFS_BAD_LEVEL			(-3)
#define BTRFS_BAD_NRITEMS		(-4)

/* Calculate max possible nritems for a leaf/node */
static u32 max_nritems(u8 level, u32 nodesize)
{

	if (level == 0)
		return ((nodesize - sizeof(struct btrfs_header)) /
			sizeof(struct btrfs_item));
	return ((nodesize - sizeof(struct btrfs_header)) /
		sizeof(struct btrfs_key_ptr));
}

static int check_tree_block(struct btrfs_fs_info *fs_info,
			    struct extent_buffer *buf)
{

	struct btrfs_fs_devices *fs_devices = fs_info->fs_devices;
	u32 nodesize = fs_info->nodesize;
	bool fsid_match = false;
	int ret = BTRFS_BAD_FSID;

	if (buf->start != btrfs_header_bytenr(buf))
		return BTRFS_BAD_BYTENR;
	if (btrfs_header_level(buf) >= BTRFS_MAX_LEVEL)
		return BTRFS_BAD_LEVEL;
	if (btrfs_header_nritems(buf) > max_nritems(btrfs_header_level(buf),
						    nodesize))
		return BTRFS_BAD_NRITEMS;

	/* Only leaf can be empty */
	if (btrfs_header_nritems(buf) == 0 &&
	    btrfs_header_level(buf) != 0)
		return BTRFS_BAD_NRITEMS;

	while (fs_devices) {
		/*
		 * Checking the incompat flag is only valid for the current
		 * fs. For seed devices it's forbidden to have their uuid
		 * changed so reading ->fsid in this case is fine
		 */
		if (fs_devices == fs_info->fs_devices &&
		    btrfs_fs_incompat(fs_info, METADATA_UUID))
			fsid_match = !memcmp_extent_buffer(buf,
						   fs_devices->metadata_uuid,
						   btrfs_header_fsid(),
						   BTRFS_FSID_SIZE);
		else
			fsid_match = !memcmp_extent_buffer(buf,
						    fs_devices->fsid,
						    btrfs_header_fsid(),
						    BTRFS_FSID_SIZE);


		if (fsid_match) {
			ret = 0;
			break;
		}
		fs_devices = fs_devices->seed;
	}
	return ret;
}

static void print_tree_block_error(struct btrfs_fs_info *fs_info,
				struct extent_buffer *eb,
				int err)
{
	char fs_uuid[BTRFS_UUID_UNPARSED_SIZE] = {'\0'};
	char found_uuid[BTRFS_UUID_UNPARSED_SIZE] = {'\0'};
	u8 buf[BTRFS_UUID_SIZE];

	if (!err)
		return;

	fprintf(stderr, "bad tree block %llu, ", eb->start);
	switch (err) {
	case BTRFS_BAD_FSID:
		read_extent_buffer(eb, buf, btrfs_header_fsid(),
				   BTRFS_UUID_SIZE);
		uuid_unparse(buf, found_uuid);
		uuid_unparse(fs_info->fs_devices->metadata_uuid, fs_uuid);
		fprintf(stderr, "fsid mismatch, want=%s, have=%s\n",
			fs_uuid, found_uuid);
		break;
	case BTRFS_BAD_BYTENR:
		fprintf(stderr, "bytenr mismatch, want=%llu, have=%llu\n",
			eb->start, btrfs_header_bytenr(eb));
		break;
	case BTRFS_BAD_LEVEL:
		fprintf(stderr, "bad level, %u > %d\n",
			btrfs_header_level(eb), BTRFS_MAX_LEVEL);
		break;
	case BTRFS_BAD_NRITEMS:
		fprintf(stderr, "invalid nr_items: %u\n",
			btrfs_header_nritems(eb));
		break;
	}
}

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

static int __csum_tree_block_size(struct extent_buffer *buf, u16 csum_size,
				  int verify, int silent, u16 csum_type)
{
	u8 result[BTRFS_CSUM_SIZE];
	u32 len;

	len = buf->len - BTRFS_CSUM_SIZE;
	btrfs_csum_data(csum_type, (u8 *)buf->data + BTRFS_CSUM_SIZE,
			result, len);

	if (verify) {
		if (memcmp_extent_buffer(buf, result, 0, csum_size)) {
			/* FIXME: format */
			if (!silent)
				printk("checksum verify failed on %llu found %08X wanted %08X\n",
				       (unsigned long long)buf->start,
				       result[0],
				       buf->data[0]);
			return 1;
		}
	} else {
		write_extent_buffer(buf, result, 0, csum_size);
	}
	return 0;
}

int csum_tree_block_size(struct extent_buffer *buf, u16 csum_size, int verify,
			 u16 csum_type)
{
	return __csum_tree_block_size(buf, csum_size, verify, 0, csum_type);
}

static int csum_tree_block(struct btrfs_fs_info *fs_info,
			   struct extent_buffer *buf, int verify)
{
	u16 csum_size = btrfs_super_csum_size(fs_info->super_copy);
	u16 csum_type = btrfs_super_csum_type(fs_info->super_copy);

	return csum_tree_block_size(buf, csum_size, verify, csum_type);
}

struct extent_buffer *btrfs_find_tree_block(struct btrfs_fs_info *fs_info,
					    u64 bytenr, u32 blocksize)
{
	return find_extent_buffer(&fs_info->extent_cache,
				  bytenr, blocksize);
}

struct extent_buffer* btrfs_find_create_tree_block(
		struct btrfs_fs_info *fs_info, u64 bytenr)
{
	return alloc_extent_buffer(fs_info, bytenr, fs_info->nodesize);
}

static int verify_parent_transid(struct extent_io_tree *io_tree,
				 struct extent_buffer *eb, u64 parent_transid,
				 int ignore)
{
	int ret;

	if (!parent_transid || btrfs_header_generation(eb) == parent_transid)
		return 0;

	if (extent_buffer_uptodate(eb) &&
	    btrfs_header_generation(eb) == parent_transid) {
		ret = 0;
		goto out;
	}
	printk("parent transid verify failed on %llu wanted %llu found %llu\n",
	       (unsigned long long)eb->start,
	       (unsigned long long)parent_transid,
	       (unsigned long long)btrfs_header_generation(eb));
	if (ignore) {
		eb->flags |= EXTENT_BAD_TRANSID;
		printk("Ignoring transid failure\n");
		return 0;
	}

	ret = 1;
out:
	clear_extent_buffer_uptodate(eb);
	return ret;

}

int btrfs_buffer_uptodate(struct extent_buffer *buf, u64 parent_transid)
{
	int ret;

	ret = extent_buffer_uptodate(buf);
	if (!ret)
		return ret;

	ret = verify_parent_transid(&buf->fs_info->extent_cache, buf,
				    parent_transid, 1);
	return !ret;
}

int btrfs_set_buffer_uptodate(struct extent_buffer *eb)
{
	return set_extent_buffer_uptodate(eb);
}

int read_whole_eb(struct btrfs_fs_info *info, struct extent_buffer *eb, int mirror)
{
	unsigned long offset = 0;
	struct btrfs_multi_bio *multi = NULL;
	struct btrfs_device *device;
	int ret = 0;
	u64 read_len;
	unsigned long bytes_left = eb->len;

	while (bytes_left) {
		read_len = bytes_left;
		device = NULL;

		ret = btrfs_map_block(info, READ, eb->start + offset,
				      &read_len, &multi, mirror, NULL);
		if (ret) {
			printk("Couldn't map the block %Lu\n", eb->start + offset);
			kfree(multi);
			return -EIO;
		}
		device = multi->stripes[0].dev;

		if (!device->desc || !device->part) {
			kfree(multi);
			return -EIO;
		}

		if (read_len > bytes_left)
			read_len = bytes_left;

		ret = read_extent_from_disk(device->desc, device->part,
					    multi->stripes[0].physical, eb,
					    offset, read_len);
		kfree(multi);
		multi = NULL;

		if (ret)
			return -EIO;
		offset += read_len;
		bytes_left -= read_len;
	}
	return 0;
}

struct extent_buffer* read_tree_block(struct btrfs_fs_info *fs_info, u64 bytenr,
		u64 parent_transid)
{
	int ret;
	struct extent_buffer *eb;
	u64 best_transid = 0;
	u32 sectorsize = fs_info->sectorsize;
	int mirror_num = 1;
	int good_mirror = 0;
	int candidate_mirror = 0;
	int num_copies;
	int ignore = 0;

	/*
	 * Don't even try to create tree block for unaligned tree block
	 * bytenr.
	 * Such unaligned tree block will free overlapping extent buffer,
	 * causing use-after-free bugs for fuzzed images.
	 */
	if (bytenr < sectorsize || !IS_ALIGNED(bytenr, sectorsize)) {
		error("tree block bytenr %llu is not aligned to sectorsize %u",
		      bytenr, sectorsize);
		return ERR_PTR(-EIO);
	}

	eb = btrfs_find_create_tree_block(fs_info, bytenr);
	if (!eb)
		return ERR_PTR(-ENOMEM);

	if (btrfs_buffer_uptodate(eb, parent_transid))
		return eb;

	num_copies = btrfs_num_copies(fs_info, eb->start, eb->len);
	while (1) {
		ret = read_whole_eb(fs_info, eb, mirror_num);
		if (ret == 0 && csum_tree_block(fs_info, eb, 1) == 0 &&
		    check_tree_block(fs_info, eb) == 0 &&
		    verify_parent_transid(&fs_info->extent_cache, eb,
					  parent_transid, ignore) == 0) {
			/*
			 * check_tree_block() is less strict to allow btrfs
			 * check to get raw eb with bad key order and fix it.
			 * But we still need to try to get a good copy if
			 * possible, or bad key order can go into tools like
			 * btrfs ins dump-tree.
			 */
			if (btrfs_header_level(eb))
				ret = btrfs_check_node(fs_info, NULL, eb);
			else
				ret = btrfs_check_leaf(fs_info, NULL, eb);
			if (!ret || candidate_mirror == mirror_num) {
				btrfs_set_buffer_uptodate(eb);
				return eb;
			}
			if (candidate_mirror <= 0)
				candidate_mirror = mirror_num;
		}
		if (ignore) {
			if (candidate_mirror > 0) {
				mirror_num = candidate_mirror;
				continue;
			}
			if (check_tree_block(fs_info, eb))
				print_tree_block_error(fs_info, eb,
						check_tree_block(fs_info, eb));
			else
				fprintf(stderr, "Csum didn't match\n");
			ret = -EIO;
			break;
		}
		if (num_copies == 1) {
			ignore = 1;
			continue;
		}
		if (btrfs_header_generation(eb) > best_transid) {
			best_transid = btrfs_header_generation(eb);
			good_mirror = mirror_num;
		}
		mirror_num++;
		if (mirror_num > num_copies) {
			if (candidate_mirror > 0)
				mirror_num = candidate_mirror;
			else
				mirror_num = good_mirror;
			ignore = 1;
			continue;
		}
	}
	/*
	 * We failed to read this tree block, it be should deleted right now
	 * to avoid stale cache populate the cache.
	 */
	free_extent_buffer(eb);
	return ERR_PTR(ret);
}
