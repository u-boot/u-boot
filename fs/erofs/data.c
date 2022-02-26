// SPDX-License-Identifier: GPL-2.0+
#include "internal.h"
#include "decompress.h"

static int erofs_map_blocks_flatmode(struct erofs_inode *inode,
				     struct erofs_map_blocks *map,
				     int flags)
{
	int err = 0;
	erofs_blk_t nblocks, lastblk;
	u64 offset = map->m_la;
	struct erofs_inode *vi = inode;
	bool tailendpacking = (vi->datalayout == EROFS_INODE_FLAT_INLINE);

	nblocks = DIV_ROUND_UP(inode->i_size, EROFS_BLKSIZ);
	lastblk = nblocks - tailendpacking;

	/* there is no hole in flatmode */
	map->m_flags = EROFS_MAP_MAPPED;

	if (offset < blknr_to_addr(lastblk)) {
		map->m_pa = blknr_to_addr(vi->u.i_blkaddr) + map->m_la;
		map->m_plen = blknr_to_addr(lastblk) - offset;
	} else if (tailendpacking) {
		/* 2 - inode inline B: inode, [xattrs], inline last blk... */
		map->m_pa = iloc(vi->nid) + vi->inode_isize +
			vi->xattr_isize + erofs_blkoff(map->m_la);
		map->m_plen = inode->i_size - offset;

		/* inline data should be located in one meta block */
		if (erofs_blkoff(map->m_pa) + map->m_plen > PAGE_SIZE) {
			erofs_err("inline data cross block boundary @ nid %" PRIu64,
				  vi->nid);
			DBG_BUGON(1);
			err = -EFSCORRUPTED;
			goto err_out;
		}

		map->m_flags |= EROFS_MAP_META;
	} else {
		erofs_err("internal error @ nid: %" PRIu64 " (size %llu), m_la 0x%" PRIx64,
			  vi->nid, (unsigned long long)inode->i_size, map->m_la);
		DBG_BUGON(1);
		err = -EIO;
		goto err_out;
	}

	map->m_llen = map->m_plen;
err_out:
	return err;
}

int erofs_map_blocks(struct erofs_inode *inode,
		     struct erofs_map_blocks *map, int flags)
{
	struct erofs_inode *vi = inode;
	struct erofs_inode_chunk_index *idx;
	u8 buf[EROFS_BLKSIZ];
	u64 chunknr;
	unsigned int unit;
	erofs_off_t pos;
	int err = 0;

	map->m_deviceid = 0;
	if (map->m_la >= inode->i_size) {
		/* leave out-of-bound access unmapped */
		map->m_flags = 0;
		map->m_plen = 0;
		goto out;
	}

	if (vi->datalayout != EROFS_INODE_CHUNK_BASED)
		return erofs_map_blocks_flatmode(inode, map, flags);

	if (vi->u.chunkformat & EROFS_CHUNK_FORMAT_INDEXES)
		unit = sizeof(*idx);			/* chunk index */
	else
		unit = EROFS_BLOCK_MAP_ENTRY_SIZE;	/* block map */

	chunknr = map->m_la >> vi->u.chunkbits;
	pos = roundup(iloc(vi->nid) + vi->inode_isize +
		      vi->xattr_isize, unit) + unit * chunknr;

	err = erofs_blk_read(buf, erofs_blknr(pos), 1);
	if (err < 0)
		return -EIO;

	map->m_la = chunknr << vi->u.chunkbits;
	map->m_plen = min_t(erofs_off_t, 1UL << vi->u.chunkbits,
			    roundup(inode->i_size - map->m_la, EROFS_BLKSIZ));

	/* handle block map */
	if (!(vi->u.chunkformat & EROFS_CHUNK_FORMAT_INDEXES)) {
		__le32 *blkaddr = (void *)buf + erofs_blkoff(pos);

		if (le32_to_cpu(*blkaddr) == EROFS_NULL_ADDR) {
			map->m_flags = 0;
		} else {
			map->m_pa = blknr_to_addr(le32_to_cpu(*blkaddr));
			map->m_flags = EROFS_MAP_MAPPED;
		}
		goto out;
	}
	/* parse chunk indexes */
	idx = (void *)buf + erofs_blkoff(pos);
	switch (le32_to_cpu(idx->blkaddr)) {
	case EROFS_NULL_ADDR:
		map->m_flags = 0;
		break;
	default:
		map->m_deviceid = le16_to_cpu(idx->device_id) &
			sbi.device_id_mask;
		map->m_pa = blknr_to_addr(le32_to_cpu(idx->blkaddr));
		map->m_flags = EROFS_MAP_MAPPED;
		break;
	}
out:
	map->m_llen = map->m_plen;
	return err;
}

int erofs_map_dev(struct erofs_sb_info *sbi, struct erofs_map_dev *map)
{
	struct erofs_device_info *dif;
	int id;

	if (map->m_deviceid) {
		if (sbi->extra_devices < map->m_deviceid)
			return -ENODEV;
	} else if (sbi->extra_devices) {
		for (id = 0; id < sbi->extra_devices; ++id) {
			erofs_off_t startoff, length;

			dif = sbi->devs + id;
			if (!dif->mapped_blkaddr)
				continue;
			startoff = blknr_to_addr(dif->mapped_blkaddr);
			length = blknr_to_addr(dif->blocks);

			if (map->m_pa >= startoff &&
			    map->m_pa < startoff + length) {
				map->m_pa -= startoff;
				break;
			}
		}
	}
	return 0;
}

static int erofs_read_raw_data(struct erofs_inode *inode, char *buffer,
			       erofs_off_t size, erofs_off_t offset)
{
	struct erofs_map_blocks map = {
		.index = UINT_MAX,
	};
	struct erofs_map_dev mdev;
	int ret;
	erofs_off_t ptr = offset;

	while (ptr < offset + size) {
		char *const estart = buffer + ptr - offset;
		erofs_off_t eend;

		map.m_la = ptr;
		ret = erofs_map_blocks(inode, &map, 0);
		if (ret)
			return ret;

		DBG_BUGON(map.m_plen != map.m_llen);

		mdev = (struct erofs_map_dev) {
			.m_deviceid = map.m_deviceid,
			.m_pa = map.m_pa,
		};
		ret = erofs_map_dev(&sbi, &mdev);
		if (ret)
			return ret;

		/* trim extent */
		eend = min(offset + size, map.m_la + map.m_llen);
		DBG_BUGON(ptr < map.m_la);

		if (!(map.m_flags & EROFS_MAP_MAPPED)) {
			if (!map.m_llen) {
				/* reached EOF */
				memset(estart, 0, offset + size - ptr);
				ptr = offset + size;
				continue;
			}
			memset(estart, 0, eend - ptr);
			ptr = eend;
			continue;
		}

		if (ptr > map.m_la) {
			mdev.m_pa += ptr - map.m_la;
			map.m_la = ptr;
		}

		ret = erofs_dev_read(mdev.m_deviceid, estart, mdev.m_pa,
				     eend - map.m_la);
		if (ret < 0)
			return -EIO;
		ptr = eend;
	}
	return 0;
}

static int z_erofs_read_data(struct erofs_inode *inode, char *buffer,
			     erofs_off_t size, erofs_off_t offset)
{
	erofs_off_t end, length, skip;
	struct erofs_map_blocks map = {
		.index = UINT_MAX,
	};
	struct erofs_map_dev mdev;
	bool partial;
	unsigned int bufsize = 0;
	char *raw = NULL;
	int ret = 0;

	end = offset + size;
	while (end > offset) {
		map.m_la = end - 1;

		ret = z_erofs_map_blocks_iter(inode, &map, 0);
		if (ret)
			break;

		/* no device id here, thus it will always succeed */
		mdev = (struct erofs_map_dev) {
			.m_pa = map.m_pa,
		};
		ret = erofs_map_dev(&sbi, &mdev);
		if (ret) {
			DBG_BUGON(1);
			break;
		}

		/*
		 * trim to the needed size if the returned extent is quite
		 * larger than requested, and set up partial flag as well.
		 */
		if (end < map.m_la + map.m_llen) {
			length = end - map.m_la;
			partial = true;
		} else {
			DBG_BUGON(end != map.m_la + map.m_llen);
			length = map.m_llen;
			partial = !(map.m_flags & EROFS_MAP_FULL_MAPPED);
		}

		if (map.m_la < offset) {
			skip = offset - map.m_la;
			end = offset;
		} else {
			skip = 0;
			end = map.m_la;
		}

		if (!(map.m_flags & EROFS_MAP_MAPPED)) {
			memset(buffer + end - offset, 0, length);
			end = map.m_la;
			continue;
		}

		if (map.m_plen > bufsize) {
			bufsize = map.m_plen;
			raw = realloc(raw, bufsize);
			if (!raw) {
				ret = -ENOMEM;
				break;
			}
		}
		ret = erofs_dev_read(mdev.m_deviceid, raw, mdev.m_pa, map.m_plen);
		if (ret < 0)
			break;

		ret = z_erofs_decompress(&(struct z_erofs_decompress_req) {
					.in = raw,
					.out = buffer + end - offset,
					.decodedskip = skip,
					.inputsize = map.m_plen,
					.decodedlength = length,
					.alg = map.m_algorithmformat,
					.partial_decoding = partial
					 });
		if (ret < 0)
			break;
	}
	if (raw)
		free(raw);
	return ret < 0 ? ret : 0;
}

int erofs_pread(struct erofs_inode *inode, char *buf,
		erofs_off_t count, erofs_off_t offset)
{
	switch (inode->datalayout) {
	case EROFS_INODE_FLAT_PLAIN:
	case EROFS_INODE_FLAT_INLINE:
	case EROFS_INODE_CHUNK_BASED:
		return erofs_read_raw_data(inode, buf, count, offset);
	case EROFS_INODE_FLAT_COMPRESSION_LEGACY:
	case EROFS_INODE_FLAT_COMPRESSION:
		return z_erofs_read_data(inode, buf, count, offset);
	default:
		break;
	}
	return -EINVAL;
}
