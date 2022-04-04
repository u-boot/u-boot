// SPDX-License-Identifier: GPL-2.0+
#include "internal.h"

static bool check_layout_compatibility(struct erofs_sb_info *sbi,
				       struct erofs_super_block *dsb)
{
	const unsigned int feature = le32_to_cpu(dsb->feature_incompat);

	sbi->feature_incompat = feature;

	/* check if current kernel meets all mandatory requirements */
	if (feature & (~EROFS_ALL_FEATURE_INCOMPAT)) {
		erofs_err("unidentified incompatible feature %x, please upgrade kernel version",
			  feature & ~EROFS_ALL_FEATURE_INCOMPAT);
		return false;
	}
	return true;
}

static int erofs_init_devices(struct erofs_sb_info *sbi,
			      struct erofs_super_block *dsb)
{
	unsigned int ondisk_extradevs, i;
	erofs_off_t pos;

	sbi->total_blocks = sbi->primarydevice_blocks;

	if (!erofs_sb_has_device_table())
		ondisk_extradevs = 0;
	else
		ondisk_extradevs = le16_to_cpu(dsb->extra_devices);

	if (ondisk_extradevs != sbi->extra_devices) {
		erofs_err("extra devices don't match (ondisk %u, given %u)",
			  ondisk_extradevs, sbi->extra_devices);
		return -EINVAL;
	}
	if (!ondisk_extradevs)
		return 0;

	sbi->device_id_mask = roundup_pow_of_two(ondisk_extradevs + 1) - 1;
	sbi->devs = calloc(ondisk_extradevs, sizeof(*sbi->devs));
	pos = le16_to_cpu(dsb->devt_slotoff) * EROFS_DEVT_SLOT_SIZE;
	for (i = 0; i < ondisk_extradevs; ++i) {
		struct erofs_deviceslot dis;
		int ret;

		ret = erofs_dev_read(0, &dis, pos, sizeof(dis));
		if (ret < 0)
			return ret;

		sbi->devs[i].mapped_blkaddr = dis.mapped_blkaddr;
		sbi->total_blocks += dis.blocks;
		pos += EROFS_DEVT_SLOT_SIZE;
	}
	return 0;
}

int erofs_read_superblock(void)
{
	char data[EROFS_BLKSIZ];
	struct erofs_super_block *dsb;
	unsigned int blkszbits;
	int ret;

	ret = erofs_blk_read(data, 0, 1);
	if (ret < 0) {
		erofs_err("cannot read erofs superblock: %d", ret);
		return -EIO;
	}
	dsb = (struct erofs_super_block *)(data + EROFS_SUPER_OFFSET);

	ret = -EINVAL;
	if (le32_to_cpu(dsb->magic) != EROFS_SUPER_MAGIC_V1) {
		erofs_err("cannot find valid erofs superblock");
		return ret;
	}

	sbi.feature_compat = le32_to_cpu(dsb->feature_compat);

	blkszbits = dsb->blkszbits;
	/* 9(512 bytes) + LOG_SECTORS_PER_BLOCK == LOG_BLOCK_SIZE */
	if (blkszbits != LOG_BLOCK_SIZE) {
		erofs_err("blksize %u isn't supported on this platform",
			  1 << blkszbits);
		return ret;
	}

	if (!check_layout_compatibility(&sbi, dsb))
		return ret;

	sbi.primarydevice_blocks = le32_to_cpu(dsb->blocks);
	sbi.meta_blkaddr = le32_to_cpu(dsb->meta_blkaddr);
	sbi.xattr_blkaddr = le32_to_cpu(dsb->xattr_blkaddr);
	sbi.islotbits = EROFS_ISLOTBITS;
	sbi.root_nid = le16_to_cpu(dsb->root_nid);
	sbi.inos = le64_to_cpu(dsb->inos);
	sbi.checksum = le32_to_cpu(dsb->checksum);

	sbi.build_time = le64_to_cpu(dsb->build_time);
	sbi.build_time_nsec = le32_to_cpu(dsb->build_time_nsec);

	memcpy(&sbi.uuid, dsb->uuid, sizeof(dsb->uuid));
	return erofs_init_devices(&sbi, dsb);
}
