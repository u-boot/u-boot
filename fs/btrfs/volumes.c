// SPDX-License-Identifier: GPL-2.0+
#include <stdlib.h>
#include <common.h>
#include <fs_internal.h>
#include "ctree.h"
#include "disk-io.h"
#include "volumes.h"

const struct btrfs_raid_attr btrfs_raid_array[BTRFS_NR_RAID_TYPES] = {
	[BTRFS_RAID_RAID10] = {
		.sub_stripes	= 2,
		.dev_stripes	= 1,
		.devs_max	= 0,	/* 0 == as many as possible */
		.devs_min	= 4,
		.tolerated_failures = 1,
		.devs_increment	= 2,
		.ncopies	= 2,
		.nparity	= 0,
		.raid_name	= "raid10",
		.bg_flag	= BTRFS_BLOCK_GROUP_RAID10,
	},
	[BTRFS_RAID_RAID1] = {
		.sub_stripes	= 1,
		.dev_stripes	= 1,
		.devs_max	= 2,
		.devs_min	= 2,
		.tolerated_failures = 1,
		.devs_increment	= 2,
		.ncopies	= 2,
		.nparity	= 0,
		.raid_name	= "raid1",
		.bg_flag	= BTRFS_BLOCK_GROUP_RAID1,
	},
	[BTRFS_RAID_RAID1C3] = {
		.sub_stripes	= 1,
		.dev_stripes	= 1,
		.devs_max	= 3,
		.devs_min	= 3,
		.tolerated_failures = 2,
		.devs_increment	= 3,
		.ncopies	= 3,
		.raid_name	= "raid1c3",
		.bg_flag	= BTRFS_BLOCK_GROUP_RAID1C3,
	},
	[BTRFS_RAID_RAID1C4] = {
		.sub_stripes	= 1,
		.dev_stripes	= 1,
		.devs_max	= 4,
		.devs_min	= 4,
		.tolerated_failures = 3,
		.devs_increment	= 4,
		.ncopies	= 4,
		.raid_name	= "raid1c4",
		.bg_flag	= BTRFS_BLOCK_GROUP_RAID1C4,
	},
	[BTRFS_RAID_DUP] = {
		.sub_stripes	= 1,
		.dev_stripes	= 2,
		.devs_max	= 1,
		.devs_min	= 1,
		.tolerated_failures = 0,
		.devs_increment	= 1,
		.ncopies	= 2,
		.nparity	= 0,
		.raid_name	= "dup",
		.bg_flag	= BTRFS_BLOCK_GROUP_DUP,
	},
	[BTRFS_RAID_RAID0] = {
		.sub_stripes	= 1,
		.dev_stripes	= 1,
		.devs_max	= 0,
		.devs_min	= 2,
		.tolerated_failures = 0,
		.devs_increment	= 1,
		.ncopies	= 1,
		.nparity	= 0,
		.raid_name	= "raid0",
		.bg_flag	= BTRFS_BLOCK_GROUP_RAID0,
	},
	[BTRFS_RAID_SINGLE] = {
		.sub_stripes	= 1,
		.dev_stripes	= 1,
		.devs_max	= 1,
		.devs_min	= 1,
		.tolerated_failures = 0,
		.devs_increment	= 1,
		.ncopies	= 1,
		.nparity	= 0,
		.raid_name	= "single",
		.bg_flag	= 0,
	},
	[BTRFS_RAID_RAID5] = {
		.sub_stripes	= 1,
		.dev_stripes	= 1,
		.devs_max	= 0,
		.devs_min	= 2,
		.tolerated_failures = 1,
		.devs_increment	= 1,
		.ncopies	= 1,
		.nparity	= 1,
		.raid_name	= "raid5",
		.bg_flag	= BTRFS_BLOCK_GROUP_RAID5,
	},
	[BTRFS_RAID_RAID6] = {
		.sub_stripes	= 1,
		.dev_stripes	= 1,
		.devs_max	= 0,
		.devs_min	= 3,
		.tolerated_failures = 2,
		.devs_increment	= 1,
		.ncopies	= 1,
		.nparity	= 2,
		.raid_name	= "raid6",
		.bg_flag	= BTRFS_BLOCK_GROUP_RAID6,
	},
};

struct stripe {
	struct btrfs_device *dev;
	u64 physical;
};

static inline int nr_parity_stripes(struct map_lookup *map)
{
	if (map->type & BTRFS_BLOCK_GROUP_RAID5)
		return 1;
	else if (map->type & BTRFS_BLOCK_GROUP_RAID6)
		return 2;
	else
		return 0;
}

static inline int nr_data_stripes(struct map_lookup *map)
{
	return map->num_stripes - nr_parity_stripes(map);
}

#define is_parity_stripe(x) ( ((x) == BTRFS_RAID5_P_STRIPE) || ((x) == BTRFS_RAID6_Q_STRIPE) )

static LIST_HEAD(fs_uuids);

/*
 * Find a device specified by @devid or @uuid in the list of @fs_devices, or
 * return NULL.
 *
 * If devid and uuid are both specified, the match must be exact, otherwise
 * only devid is used.
 */
static struct btrfs_device *find_device(struct btrfs_fs_devices *fs_devices,
		u64 devid, u8 *uuid)
{
	struct list_head *head = &fs_devices->devices;
	struct btrfs_device *dev;

	list_for_each_entry(dev, head, dev_list) {
		if (dev->devid == devid &&
		    (!uuid || !memcmp(dev->uuid, uuid, BTRFS_UUID_SIZE))) {
			return dev;
		}
	}
	return NULL;
}

static struct btrfs_fs_devices *find_fsid(u8 *fsid, u8 *metadata_uuid)
{
	struct btrfs_fs_devices *fs_devices;

	list_for_each_entry(fs_devices, &fs_uuids, list) {
		if (metadata_uuid && (memcmp(fsid, fs_devices->fsid,
					     BTRFS_FSID_SIZE) == 0) &&
		    (memcmp(metadata_uuid, fs_devices->metadata_uuid,
			    BTRFS_FSID_SIZE) == 0)) {
			return fs_devices;
		} else if (memcmp(fsid, fs_devices->fsid, BTRFS_FSID_SIZE) == 0){
			return fs_devices;
		}
	}
	return NULL;
}

static int device_list_add(struct btrfs_super_block *disk_super,
			   u64 devid, struct blk_desc *desc,
			   struct disk_partition *part,
			   struct btrfs_fs_devices **fs_devices_ret)
{
	struct btrfs_device *device;
	struct btrfs_fs_devices *fs_devices;
	u64 found_transid = btrfs_super_generation(disk_super);
	bool metadata_uuid = (btrfs_super_incompat_flags(disk_super) &
		BTRFS_FEATURE_INCOMPAT_METADATA_UUID);

	if (metadata_uuid)
		fs_devices = find_fsid(disk_super->fsid,
				       disk_super->metadata_uuid);
	else
		fs_devices = find_fsid(disk_super->fsid, NULL);

	if (!fs_devices) {
		fs_devices = kzalloc(sizeof(*fs_devices), GFP_NOFS);
		if (!fs_devices)
			return -ENOMEM;
		INIT_LIST_HEAD(&fs_devices->devices);
		list_add(&fs_devices->list, &fs_uuids);
		memcpy(fs_devices->fsid, disk_super->fsid, BTRFS_FSID_SIZE);
		if (metadata_uuid)
			memcpy(fs_devices->metadata_uuid,
			       disk_super->metadata_uuid, BTRFS_FSID_SIZE);
		else
			memcpy(fs_devices->metadata_uuid, fs_devices->fsid,
			       BTRFS_FSID_SIZE);

		fs_devices->latest_devid = devid;
		fs_devices->latest_trans = found_transid;
		fs_devices->lowest_devid = (u64)-1;
		device = NULL;
	} else {
		device = find_device(fs_devices, devid,
				    disk_super->dev_item.uuid);
	}
	if (!device) {
		device = kzalloc(sizeof(*device), GFP_NOFS);
		if (!device) {
			/* we can safely leave the fs_devices entry around */
			return -ENOMEM;
		}
		device->devid = devid;
		device->desc = desc;
		device->part = part;
		device->generation = found_transid;
		memcpy(device->uuid, disk_super->dev_item.uuid,
		       BTRFS_UUID_SIZE);
		device->total_devs = btrfs_super_num_devices(disk_super);
		device->super_bytes_used = btrfs_super_bytes_used(disk_super);
		device->total_bytes =
			btrfs_stack_device_total_bytes(&disk_super->dev_item);
		device->bytes_used =
			btrfs_stack_device_bytes_used(&disk_super->dev_item);
		list_add(&device->dev_list, &fs_devices->devices);
		device->fs_devices = fs_devices;
	} else if (!device->desc || !device->part) {
		/*
		 * The existing device has newer generation, so this one could
		 * be a stale one, don't add it.
		 */
		if (found_transid < device->generation) {
			error(
	"adding devid %llu gen %llu but found an existing device gen %llu",
				device->devid, found_transid,
				device->generation);
			return -EEXIST;
		} else {
			device->desc = desc;
			device->part = part;
		}
	}


	if (found_transid > fs_devices->latest_trans) {
		fs_devices->latest_devid = devid;
		fs_devices->latest_trans = found_transid;
	}
	if (fs_devices->lowest_devid > devid) {
		fs_devices->lowest_devid = devid;
	}
	*fs_devices_ret = fs_devices;
	return 0;
}

int btrfs_close_devices(struct btrfs_fs_devices *fs_devices)
{
	struct btrfs_fs_devices *seed_devices;
	struct btrfs_device *device;
	int ret = 0;

again:
	if (!fs_devices)
		return 0;
	while (!list_empty(&fs_devices->devices)) {
		device = list_entry(fs_devices->devices.next,
				    struct btrfs_device, dev_list);
		list_del(&device->dev_list);
		/* free the memory */
		free(device);
	}

	seed_devices = fs_devices->seed;
	fs_devices->seed = NULL;
	if (seed_devices) {
		struct btrfs_fs_devices *orig;

		orig = fs_devices;
		fs_devices = seed_devices;
		list_del(&orig->list);
		free(orig);
		goto again;
	} else {
		list_del(&fs_devices->list);
		free(fs_devices);
	}

	return ret;
}

void btrfs_close_all_devices(void)
{
	struct btrfs_fs_devices *fs_devices;

	while (!list_empty(&fs_uuids)) {
		fs_devices = list_entry(fs_uuids.next, struct btrfs_fs_devices,
					list);
		btrfs_close_devices(fs_devices);
	}
}

int btrfs_open_devices(struct btrfs_fs_devices *fs_devices)
{
	struct btrfs_device *device;

	list_for_each_entry(device, &fs_devices->devices, dev_list) {
		if (!device->desc || !device->part) {
			printf("no device found for devid %llu, skip it \n",
				device->devid);
			continue;
		}
	}
	return 0;
}

int btrfs_scan_one_device(struct blk_desc *desc, struct disk_partition *part,
			  struct btrfs_fs_devices **fs_devices_ret,
			  u64 *total_devs)
{
	struct btrfs_super_block *disk_super;
	char buf[BTRFS_SUPER_INFO_SIZE];
	int ret;
	u64 devid;

	disk_super = (struct btrfs_super_block *)buf;
	ret = btrfs_read_dev_super(desc, part, disk_super);
	if (ret < 0)
		return -EIO;
	devid = btrfs_stack_device_id(&disk_super->dev_item);
	if (btrfs_super_flags(disk_super) & BTRFS_SUPER_FLAG_METADUMP)
		*total_devs = 1;
	else
		*total_devs = btrfs_super_num_devices(disk_super);

	ret = device_list_add(disk_super, devid, desc, part, fs_devices_ret);

	return ret;
}

struct btrfs_device *btrfs_find_device(struct btrfs_fs_info *fs_info, u64 devid,
				       u8 *uuid, u8 *fsid)
{
	struct btrfs_device *device;
	struct btrfs_fs_devices *cur_devices;

	cur_devices = fs_info->fs_devices;
	while (cur_devices) {
		if (!fsid ||
		   !memcmp(cur_devices->metadata_uuid, fsid, BTRFS_FSID_SIZE)) {
			device = find_device(cur_devices, devid, uuid);
			if (device)
				return device;
		}
		cur_devices = cur_devices->seed;
	}
	return NULL;
}

/*
 * slot == -1: SYSTEM chunk
 * return -EIO on error, otherwise return 0
 */
int btrfs_check_chunk_valid(struct btrfs_fs_info *fs_info,
			    struct extent_buffer *leaf,
			    struct btrfs_chunk *chunk,
			    int slot, u64 logical)
{
	u64 length;
	u64 stripe_len;
	u16 num_stripes;
	u16 sub_stripes;
	u64 type;
	u32 chunk_ondisk_size;
	u32 sectorsize = fs_info->sectorsize;

	/*
	 * Basic chunk item size check.  Note that btrfs_chunk already contains
	 * one stripe, so no "==" check.
	 */
	if (slot >= 0 &&
	    btrfs_item_size_nr(leaf, slot) < sizeof(struct btrfs_chunk)) {
		error("invalid chunk item size, have %u expect [%zu, %zu)",
			btrfs_item_size_nr(leaf, slot),
			sizeof(struct btrfs_chunk),
			BTRFS_LEAF_DATA_SIZE(fs_info));
		return -EUCLEAN;
	}
	length = btrfs_chunk_length(leaf, chunk);
	stripe_len = btrfs_chunk_stripe_len(leaf, chunk);
	num_stripes = btrfs_chunk_num_stripes(leaf, chunk);
	sub_stripes = btrfs_chunk_sub_stripes(leaf, chunk);
	type = btrfs_chunk_type(leaf, chunk);

	if (num_stripes == 0) {
		error("invalid num_stripes, have %u expect non-zero",
			num_stripes);
		return -EUCLEAN;
	}
	if (slot >= 0 && btrfs_chunk_item_size(num_stripes) !=
	    btrfs_item_size_nr(leaf, slot)) {
		error("invalid chunk item size, have %u expect %lu",
			btrfs_item_size_nr(leaf, slot),
			btrfs_chunk_item_size(num_stripes));
		return -EUCLEAN;
	}

	/*
	 * These valid checks may be insufficient to cover every corner cases.
	 */
	if (!IS_ALIGNED(logical, sectorsize)) {
		error("invalid chunk logical %llu",  logical);
		return -EIO;
	}
	if (btrfs_chunk_sector_size(leaf, chunk) != sectorsize) {
		error("invalid chunk sectorsize %llu",
		      (unsigned long long)btrfs_chunk_sector_size(leaf, chunk));
		return -EIO;
	}
	if (!length || !IS_ALIGNED(length, sectorsize)) {
		error("invalid chunk length %llu",  length);
		return -EIO;
	}
	if (stripe_len != BTRFS_STRIPE_LEN) {
		error("invalid chunk stripe length: %llu", stripe_len);
		return -EIO;
	}
	/* Check on chunk item type */
	if (slot == -1 && (type & BTRFS_BLOCK_GROUP_SYSTEM) == 0) {
		error("invalid chunk type %llu", type);
		return -EIO;
	}
	if (type & ~(BTRFS_BLOCK_GROUP_TYPE_MASK |
		     BTRFS_BLOCK_GROUP_PROFILE_MASK)) {
		error("unrecognized chunk type: %llu",
		      ~(BTRFS_BLOCK_GROUP_TYPE_MASK |
			BTRFS_BLOCK_GROUP_PROFILE_MASK) & type);
		return -EIO;
	}
	if (!(type & BTRFS_BLOCK_GROUP_TYPE_MASK)) {
		error("missing chunk type flag: %llu", type);
		return -EIO;
	}
	if (!(is_power_of_2(type & BTRFS_BLOCK_GROUP_PROFILE_MASK) ||
	      (type & BTRFS_BLOCK_GROUP_PROFILE_MASK) == 0)) {
		error("conflicting chunk type detected: %llu", type);
		return -EIO;
	}
	if ((type & BTRFS_BLOCK_GROUP_PROFILE_MASK) &&
	    !is_power_of_2(type & BTRFS_BLOCK_GROUP_PROFILE_MASK)) {
		error("conflicting chunk profile detected: %llu", type);
		return -EIO;
	}

	chunk_ondisk_size = btrfs_chunk_item_size(num_stripes);
	/*
	 * Btrfs_chunk contains at least one stripe, and for sys_chunk
	 * it can't exceed the system chunk array size
	 * For normal chunk, it should match its chunk item size.
	 */
	if (num_stripes < 1 ||
	    (slot == -1 && chunk_ondisk_size > BTRFS_SYSTEM_CHUNK_ARRAY_SIZE) ||
	    (slot >= 0 && chunk_ondisk_size > btrfs_item_size_nr(leaf, slot))) {
		error("invalid num_stripes: %u", num_stripes);
		return -EIO;
	}
	/*
	 * Device number check against profile
	 */
	if ((type & BTRFS_BLOCK_GROUP_RAID10 && (sub_stripes != 2 ||
		  !IS_ALIGNED(num_stripes, sub_stripes))) ||
	    (type & BTRFS_BLOCK_GROUP_RAID1 && num_stripes < 1) ||
	    (type & BTRFS_BLOCK_GROUP_RAID1C3 && num_stripes < 3) ||
	    (type & BTRFS_BLOCK_GROUP_RAID1C4 && num_stripes < 4) ||
	    (type & BTRFS_BLOCK_GROUP_RAID5 && num_stripes < 2) ||
	    (type & BTRFS_BLOCK_GROUP_RAID6 && num_stripes < 3) ||
	    (type & BTRFS_BLOCK_GROUP_DUP && num_stripes > 2) ||
	    ((type & BTRFS_BLOCK_GROUP_PROFILE_MASK) == 0 &&
	     num_stripes != 1)) {
		error("Invalid num_stripes:sub_stripes %u:%u for profile %llu",
		      num_stripes, sub_stripes,
		      type & BTRFS_BLOCK_GROUP_PROFILE_MASK);
		return -EIO;
	}

	return 0;
}

/*
 * Get stripe length from chunk item and its stripe items
 *
 * Caller should only call this function after validating the chunk item
 * by using btrfs_check_chunk_valid().
 */
u64 btrfs_stripe_length(struct btrfs_fs_info *fs_info,
			struct extent_buffer *leaf,
			struct btrfs_chunk *chunk)
{
	u64 stripe_len;
	u64 chunk_len;
	u32 num_stripes = btrfs_chunk_num_stripes(leaf, chunk);
	u64 profile = btrfs_chunk_type(leaf, chunk) &
		      BTRFS_BLOCK_GROUP_PROFILE_MASK;

	chunk_len = btrfs_chunk_length(leaf, chunk);

	switch (profile) {
	case 0: /* Single profile */
	case BTRFS_BLOCK_GROUP_RAID1:
	case BTRFS_BLOCK_GROUP_RAID1C3:
	case BTRFS_BLOCK_GROUP_RAID1C4:
	case BTRFS_BLOCK_GROUP_DUP:
		stripe_len = chunk_len;
		break;
	case BTRFS_BLOCK_GROUP_RAID0:
		stripe_len = chunk_len / num_stripes;
		break;
	case BTRFS_BLOCK_GROUP_RAID5:
		stripe_len = chunk_len / (num_stripes - 1);
		break;
	case BTRFS_BLOCK_GROUP_RAID6:
		stripe_len = chunk_len / (num_stripes - 2);
		break;
	case BTRFS_BLOCK_GROUP_RAID10:
		stripe_len = chunk_len / (num_stripes /
				btrfs_chunk_sub_stripes(leaf, chunk));
		break;
	default:
		/* Invalid chunk profile found */
		BUG_ON(1);
	}
	return stripe_len;
}

int btrfs_num_copies(struct btrfs_fs_info *fs_info, u64 logical, u64 len)
{
	struct btrfs_mapping_tree *map_tree = &fs_info->mapping_tree;
	struct cache_extent *ce;
	struct map_lookup *map;
	int ret;

	ce = search_cache_extent(&map_tree->cache_tree, logical);
	if (!ce) {
		fprintf(stderr, "No mapping for %llu-%llu\n",
			(unsigned long long)logical,
			(unsigned long long)logical+len);
		return 1;
	}
	if (ce->start > logical || ce->start + ce->size < logical) {
		fprintf(stderr, "Invalid mapping for %llu-%llu, got "
			"%llu-%llu\n", (unsigned long long)logical,
			(unsigned long long)logical+len,
			(unsigned long long)ce->start,
			(unsigned long long)ce->start + ce->size);
		return 1;
	}
	map = container_of(ce, struct map_lookup, ce);

	if (map->type & (BTRFS_BLOCK_GROUP_DUP | BTRFS_BLOCK_GROUP_RAID1 |
			 BTRFS_BLOCK_GROUP_RAID1C3 | BTRFS_BLOCK_GROUP_RAID1C4))
		ret = map->num_stripes;
	else if (map->type & BTRFS_BLOCK_GROUP_RAID10)
		ret = map->sub_stripes;
	else if (map->type & BTRFS_BLOCK_GROUP_RAID5)
		ret = 2;
	else if (map->type & BTRFS_BLOCK_GROUP_RAID6)
		ret = 3;
	else
		ret = 1;
	return ret;
}

int btrfs_next_bg(struct btrfs_fs_info *fs_info, u64 *logical,
		  u64 *size, u64 type)
{
	struct btrfs_mapping_tree *map_tree = &fs_info->mapping_tree;
	struct cache_extent *ce;
	struct map_lookup *map;
	u64 cur = *logical;

	ce = search_cache_extent(&map_tree->cache_tree, cur);

	while (ce) {
		/*
		 * only jump to next bg if our cur is not 0
		 * As the initial logical for btrfs_next_bg() is 0, and
		 * if we jump to next bg, we skipped a valid bg.
		 */
		if (cur) {
			ce = next_cache_extent(ce);
			if (!ce)
				return -ENOENT;
		}

		cur = ce->start;
		map = container_of(ce, struct map_lookup, ce);
		if (map->type & type) {
			*logical = ce->start;
			*size = ce->size;
			return 0;
		}
		if (!cur)
			ce = next_cache_extent(ce);
	}

	return -ENOENT;
}

static inline int parity_smaller(u64 a, u64 b)
{
	return a > b;
}

/* Bubble-sort the stripe set to put the parity/syndrome stripes last */
static void sort_parity_stripes(struct btrfs_multi_bio *bbio, u64 *raid_map)
{
	struct btrfs_bio_stripe s;
	int i;
	u64 l;
	int again = 1;

	while (again) {
		again = 0;
		for (i = 0; i < bbio->num_stripes - 1; i++) {
			if (parity_smaller(raid_map[i], raid_map[i+1])) {
				s = bbio->stripes[i];
				l = raid_map[i];
				bbio->stripes[i] = bbio->stripes[i+1];
				raid_map[i] = raid_map[i+1];
				bbio->stripes[i+1] = s;
				raid_map[i+1] = l;
				again = 1;
			}
		}
	}
}

int __btrfs_map_block(struct btrfs_fs_info *fs_info, int rw,
		      u64 logical, u64 *length, u64 *type,
		      struct btrfs_multi_bio **multi_ret, int mirror_num,
		      u64 **raid_map_ret)
{
	struct btrfs_mapping_tree *map_tree = &fs_info->mapping_tree;
	struct cache_extent *ce;
	struct map_lookup *map;
	u64 offset;
	u64 stripe_offset;
	u64 *raid_map = NULL;
	int stripe_nr;
	int stripes_allocated = 8;
	int stripes_required = 1;
	int stripe_index;
	int i;
	struct btrfs_multi_bio *multi = NULL;

	if (multi_ret && rw == READ) {
		stripes_allocated = 1;
	}
again:
	ce = search_cache_extent(&map_tree->cache_tree, logical);
	if (!ce) {
		kfree(multi);
		*length = (u64)-1;
		return -ENOENT;
	}
	if (ce->start > logical) {
		kfree(multi);
		*length = ce->start - logical;
		return -ENOENT;
	}

	if (multi_ret) {
		multi = kzalloc(btrfs_multi_bio_size(stripes_allocated),
				GFP_NOFS);
		if (!multi)
			return -ENOMEM;
	}
	map = container_of(ce, struct map_lookup, ce);
	offset = logical - ce->start;

	if (rw == WRITE) {
		if (map->type & (BTRFS_BLOCK_GROUP_RAID1 |
				 BTRFS_BLOCK_GROUP_RAID1C3 |
				 BTRFS_BLOCK_GROUP_RAID1C4 |
				 BTRFS_BLOCK_GROUP_DUP)) {
			stripes_required = map->num_stripes;
		} else if (map->type & BTRFS_BLOCK_GROUP_RAID10) {
			stripes_required = map->sub_stripes;
		}
	}
	if (map->type & (BTRFS_BLOCK_GROUP_RAID5 | BTRFS_BLOCK_GROUP_RAID6)
	    && multi_ret && ((rw & WRITE) || mirror_num > 1) && raid_map_ret) {
		    /* RAID[56] write or recovery. Return all stripes */
		    stripes_required = map->num_stripes;

		    /* Only allocate the map if we've already got a large enough multi_ret */
		    if (stripes_allocated >= stripes_required) {
			    raid_map = kmalloc(sizeof(u64) * map->num_stripes, GFP_NOFS);
			    if (!raid_map) {
				    kfree(multi);
				    return -ENOMEM;
			    }
		    }
	}

	/* if our multi bio struct is too small, back off and try again */
	if (multi_ret && stripes_allocated < stripes_required) {
		stripes_allocated = stripes_required;
		kfree(multi);
		multi = NULL;
		goto again;
	}
	stripe_nr = offset;
	/*
	 * stripe_nr counts the total number of stripes we have to stride
	 * to get to this block
	 */
	stripe_nr = stripe_nr / map->stripe_len;

	stripe_offset = stripe_nr * map->stripe_len;
	BUG_ON(offset < stripe_offset);

	/* stripe_offset is the offset of this block in its stripe*/
	stripe_offset = offset - stripe_offset;

	if (map->type & (BTRFS_BLOCK_GROUP_RAID0 | BTRFS_BLOCK_GROUP_RAID1 |
			 BTRFS_BLOCK_GROUP_RAID1C3 | BTRFS_BLOCK_GROUP_RAID1C4 |
			 BTRFS_BLOCK_GROUP_RAID5 | BTRFS_BLOCK_GROUP_RAID6 |
			 BTRFS_BLOCK_GROUP_RAID10 |
			 BTRFS_BLOCK_GROUP_DUP)) {
		/* we limit the length of each bio to what fits in a stripe */
		*length = min_t(u64, ce->size - offset,
			      map->stripe_len - stripe_offset);
	} else {
		*length = ce->size - offset;
	}

	if (!multi_ret)
		goto out;

	multi->num_stripes = 1;
	stripe_index = 0;
	if (map->type & (BTRFS_BLOCK_GROUP_RAID1 |
			 BTRFS_BLOCK_GROUP_RAID1C3 |
			 BTRFS_BLOCK_GROUP_RAID1C4)) {
		if (rw == WRITE)
			multi->num_stripes = map->num_stripes;
		else if (mirror_num)
			stripe_index = mirror_num - 1;
		else
			stripe_index = stripe_nr % map->num_stripes;
	} else if (map->type & BTRFS_BLOCK_GROUP_RAID10) {
		int factor = map->num_stripes / map->sub_stripes;

		stripe_index = stripe_nr % factor;
		stripe_index *= map->sub_stripes;

		if (rw == WRITE)
			multi->num_stripes = map->sub_stripes;
		else if (mirror_num)
			stripe_index += mirror_num - 1;

		stripe_nr = stripe_nr / factor;
	} else if (map->type & BTRFS_BLOCK_GROUP_DUP) {
		if (rw == WRITE)
			multi->num_stripes = map->num_stripes;
		else if (mirror_num)
			stripe_index = mirror_num - 1;
	} else if (map->type & (BTRFS_BLOCK_GROUP_RAID5 |
				BTRFS_BLOCK_GROUP_RAID6)) {

		if (raid_map) {
			int rot;
			u64 tmp;
			u64 raid56_full_stripe_start;
			u64 full_stripe_len = nr_data_stripes(map) * map->stripe_len;

			/*
			 * align the start of our data stripe in the logical
			 * address space
			 */
			raid56_full_stripe_start = offset / full_stripe_len;
			raid56_full_stripe_start *= full_stripe_len;

			/* get the data stripe number */
			stripe_nr = raid56_full_stripe_start / map->stripe_len;
			stripe_nr = stripe_nr / nr_data_stripes(map);

			/* Work out the disk rotation on this stripe-set */
			rot = stripe_nr % map->num_stripes;

			/* Fill in the logical address of each stripe */
			tmp = stripe_nr * nr_data_stripes(map);

			for (i = 0; i < nr_data_stripes(map); i++)
				raid_map[(i+rot) % map->num_stripes] =
					ce->start + (tmp + i) * map->stripe_len;

			raid_map[(i+rot) % map->num_stripes] = BTRFS_RAID5_P_STRIPE;
			if (map->type & BTRFS_BLOCK_GROUP_RAID6)
				raid_map[(i+rot+1) % map->num_stripes] = BTRFS_RAID6_Q_STRIPE;

			*length = map->stripe_len;
			stripe_index = 0;
			stripe_offset = 0;
			multi->num_stripes = map->num_stripes;
		} else {
			stripe_index = stripe_nr % nr_data_stripes(map);
			stripe_nr = stripe_nr / nr_data_stripes(map);

			/*
			 * Mirror #0 or #1 means the original data block.
			 * Mirror #2 is RAID5 parity block.
			 * Mirror #3 is RAID6 Q block.
			 */
			if (mirror_num > 1)
				stripe_index = nr_data_stripes(map) + mirror_num - 2;

			/* We distribute the parity blocks across stripes */
			stripe_index = (stripe_nr + stripe_index) % map->num_stripes;
		}
	} else {
		/*
		 * after this do_div call, stripe_nr is the number of stripes
		 * on this device we have to walk to find the data, and
		 * stripe_index is the number of our device in the stripe array
		 */
		stripe_index = stripe_nr % map->num_stripes;
		stripe_nr = stripe_nr / map->num_stripes;
	}
	BUG_ON(stripe_index >= map->num_stripes);

	for (i = 0; i < multi->num_stripes; i++) {
		multi->stripes[i].physical =
			map->stripes[stripe_index].physical + stripe_offset +
			stripe_nr * map->stripe_len;
		multi->stripes[i].dev = map->stripes[stripe_index].dev;
		stripe_index++;
	}
	*multi_ret = multi;

	if (type)
		*type = map->type;

	if (raid_map) {
		sort_parity_stripes(multi, raid_map);
		*raid_map_ret = raid_map;
	}
out:
	return 0;
}

int btrfs_map_block(struct btrfs_fs_info *fs_info, int rw,
		    u64 logical, u64 *length,
		    struct btrfs_multi_bio **multi_ret, int mirror_num,
		    u64 **raid_map_ret)
{
	return __btrfs_map_block(fs_info, rw, logical, length, NULL,
				 multi_ret, mirror_num, raid_map_ret);
}
