// SPDX-License-Identifier: GPL-2.0+
#include "internal.h"

int z_erofs_fill_inode(struct erofs_inode *vi)
{
	if (!erofs_sb_has_big_pcluster() &&
	    vi->datalayout == EROFS_INODE_FLAT_COMPRESSION_LEGACY) {
		vi->z_advise = 0;
		vi->z_algorithmtype[0] = 0;
		vi->z_algorithmtype[1] = 0;
		vi->z_logical_clusterbits = LOG_BLOCK_SIZE;

		vi->flags |= EROFS_I_Z_INITED;
	}
	return 0;
}

static int z_erofs_fill_inode_lazy(struct erofs_inode *vi)
{
	int ret;
	erofs_off_t pos;
	struct z_erofs_map_header *h;
	char buf[sizeof(struct z_erofs_map_header)];

	if (vi->flags & EROFS_I_Z_INITED)
		return 0;

	DBG_BUGON(!erofs_sb_has_big_pcluster() &&
		  vi->datalayout == EROFS_INODE_FLAT_COMPRESSION_LEGACY);
	pos = round_up(iloc(vi->nid) + vi->inode_isize + vi->xattr_isize, 8);

	ret = erofs_dev_read(0, buf, pos, sizeof(buf));
	if (ret < 0)
		return -EIO;

	h = (struct z_erofs_map_header *)buf;
	vi->z_advise = le16_to_cpu(h->h_advise);
	vi->z_algorithmtype[0] = h->h_algorithmtype & 15;
	vi->z_algorithmtype[1] = h->h_algorithmtype >> 4;

	if (vi->z_algorithmtype[0] >= Z_EROFS_COMPRESSION_MAX) {
		erofs_err("unknown compression format %u for nid %llu",
			  vi->z_algorithmtype[0], (unsigned long long)vi->nid);
		return -EOPNOTSUPP;
	}

	vi->z_logical_clusterbits = LOG_BLOCK_SIZE + (h->h_clusterbits & 7);
	if (vi->datalayout == EROFS_INODE_FLAT_COMPRESSION &&
	    !(vi->z_advise & Z_EROFS_ADVISE_BIG_PCLUSTER_1) ^
	    !(vi->z_advise & Z_EROFS_ADVISE_BIG_PCLUSTER_2)) {
		erofs_err("big pcluster head1/2 of compact indexes should be consistent for nid %llu",
			  vi->nid * 1ULL);
		return -EFSCORRUPTED;
	}
	vi->flags |= EROFS_I_Z_INITED;
	return 0;
}

struct z_erofs_maprecorder {
	struct erofs_inode *inode;
	struct erofs_map_blocks *map;
	void *kaddr;

	unsigned long lcn;
	/* compression extent information gathered */
	u8  type, headtype;
	u16 clusterofs;
	u16 delta[2];
	erofs_blk_t pblk, compressedlcs;
};

static int z_erofs_reload_indexes(struct z_erofs_maprecorder *m,
				  erofs_blk_t eblk)
{
	int ret;
	struct erofs_map_blocks *const map = m->map;
	char *mpage = map->mpage;

	if (map->index == eblk)
		return 0;

	ret = erofs_blk_read(mpage, eblk, 1);
	if (ret < 0)
		return -EIO;

	map->index = eblk;

	return 0;
}

static int legacy_load_cluster_from_disk(struct z_erofs_maprecorder *m,
					 unsigned long lcn)
{
	struct erofs_inode *const vi = m->inode;
	const erofs_off_t ibase = iloc(vi->nid);
	const erofs_off_t pos =
		Z_EROFS_VLE_LEGACY_INDEX_ALIGN(ibase + vi->inode_isize +
					       vi->xattr_isize) +
		lcn * sizeof(struct z_erofs_vle_decompressed_index);
	struct z_erofs_vle_decompressed_index *di;
	unsigned int advise, type;
	int err;

	err = z_erofs_reload_indexes(m, erofs_blknr(pos));
	if (err)
		return err;

	m->lcn = lcn;
	di = m->kaddr + erofs_blkoff(pos);

	advise = le16_to_cpu(di->di_advise);
	type = (advise >> Z_EROFS_VLE_DI_CLUSTER_TYPE_BIT) &
		((1 << Z_EROFS_VLE_DI_CLUSTER_TYPE_BITS) - 1);
	switch (type) {
	case Z_EROFS_VLE_CLUSTER_TYPE_NONHEAD:
		m->clusterofs = 1 << vi->z_logical_clusterbits;
		m->delta[0] = le16_to_cpu(di->di_u.delta[0]);
		if (m->delta[0] & Z_EROFS_VLE_DI_D0_CBLKCNT) {
			if (!(vi->z_advise & Z_EROFS_ADVISE_BIG_PCLUSTER_1)) {
				DBG_BUGON(1);
				return -EFSCORRUPTED;
			}
			m->compressedlcs = m->delta[0] &
				~Z_EROFS_VLE_DI_D0_CBLKCNT;
			m->delta[0] = 1;
		}
		m->delta[1] = le16_to_cpu(di->di_u.delta[1]);
		break;
	case Z_EROFS_VLE_CLUSTER_TYPE_PLAIN:
	case Z_EROFS_VLE_CLUSTER_TYPE_HEAD:
		m->clusterofs = le16_to_cpu(di->di_clusterofs);
		m->pblk = le32_to_cpu(di->di_u.blkaddr);
		break;
	default:
		DBG_BUGON(1);
		return -EOPNOTSUPP;
	}
	m->type = type;
	return 0;
}

static unsigned int decode_compactedbits(unsigned int lobits,
					 unsigned int lomask,
					 u8 *in, unsigned int pos, u8 *type)
{
	const unsigned int v = get_unaligned_le32(in + pos / 8) >> (pos & 7);
	const unsigned int lo = v & lomask;

	*type = (v >> lobits) & 3;
	return lo;
}

static int get_compacted_la_distance(unsigned int lclusterbits,
				     unsigned int encodebits,
				     unsigned int vcnt, u8 *in, int i)
{
	const unsigned int lomask = (1 << lclusterbits) - 1;
	unsigned int lo, d1 = 0;
	u8 type;

	DBG_BUGON(i >= vcnt);

	do {
		lo = decode_compactedbits(lclusterbits, lomask,
					  in, encodebits * i, &type);

		if (type != Z_EROFS_VLE_CLUSTER_TYPE_NONHEAD)
			return d1;
		++d1;
	} while (++i < vcnt);

	/* vcnt - 1 (Z_EROFS_VLE_CLUSTER_TYPE_NONHEAD) item */
	if (!(lo & Z_EROFS_VLE_DI_D0_CBLKCNT))
		d1 += lo - 1;
	return d1;
}

static int unpack_compacted_index(struct z_erofs_maprecorder *m,
				  unsigned int amortizedshift,
				  unsigned int eofs, bool lookahead)
{
	struct erofs_inode *const vi = m->inode;
	const unsigned int lclusterbits = vi->z_logical_clusterbits;
	const unsigned int lomask = (1 << lclusterbits) - 1;
	unsigned int vcnt, base, lo, encodebits, nblk;
	int i;
	u8 *in, type;
	bool big_pcluster;

	if (1 << amortizedshift == 4)
		vcnt = 2;
	else if (1 << amortizedshift == 2 && lclusterbits == 12)
		vcnt = 16;
	else
		return -EOPNOTSUPP;

	big_pcluster = vi->z_advise & Z_EROFS_ADVISE_BIG_PCLUSTER_1;
	encodebits = ((vcnt << amortizedshift) - sizeof(__le32)) * 8 / vcnt;
	base = round_down(eofs, vcnt << amortizedshift);
	in = m->kaddr + base;

	i = (eofs - base) >> amortizedshift;

	lo = decode_compactedbits(lclusterbits, lomask,
				  in, encodebits * i, &type);
	m->type = type;
	if (type == Z_EROFS_VLE_CLUSTER_TYPE_NONHEAD) {
		m->clusterofs = 1 << lclusterbits;

		/* figure out lookahead_distance: delta[1] if needed */
		if (lookahead)
			m->delta[1] = get_compacted_la_distance(lclusterbits,
								encodebits,
								vcnt, in, i);
		if (lo & Z_EROFS_VLE_DI_D0_CBLKCNT) {
			if (!big_pcluster) {
				DBG_BUGON(1);
				return -EFSCORRUPTED;
			}
			m->compressedlcs = lo & ~Z_EROFS_VLE_DI_D0_CBLKCNT;
			m->delta[0] = 1;
			return 0;
		} else if (i + 1 != (int)vcnt) {
			m->delta[0] = lo;
			return 0;
		}
		/*
		 * since the last lcluster in the pack is special,
		 * of which lo saves delta[1] rather than delta[0].
		 * Hence, get delta[0] by the previous lcluster indirectly.
		 */
		lo = decode_compactedbits(lclusterbits, lomask,
					  in, encodebits * (i - 1), &type);
		if (type != Z_EROFS_VLE_CLUSTER_TYPE_NONHEAD)
			lo = 0;
		else if (lo & Z_EROFS_VLE_DI_D0_CBLKCNT)
			lo = 1;
		m->delta[0] = lo + 1;
		return 0;
	}
	m->clusterofs = lo;
	m->delta[0] = 0;
	/* figout out blkaddr (pblk) for HEAD lclusters */
	if (!big_pcluster) {
		nblk = 1;
		while (i > 0) {
			--i;
			lo = decode_compactedbits(lclusterbits, lomask,
						  in, encodebits * i, &type);
			if (type == Z_EROFS_VLE_CLUSTER_TYPE_NONHEAD)
				i -= lo;

			if (i >= 0)
				++nblk;
		}
	} else {
		nblk = 0;
		while (i > 0) {
			--i;
			lo = decode_compactedbits(lclusterbits, lomask,
						  in, encodebits * i, &type);
			if (type == Z_EROFS_VLE_CLUSTER_TYPE_NONHEAD) {
				if (lo & Z_EROFS_VLE_DI_D0_CBLKCNT) {
					--i;
					nblk += lo & ~Z_EROFS_VLE_DI_D0_CBLKCNT;
					continue;
				}
				if (lo == 1) {
					DBG_BUGON(1);
					/* --i; ++nblk;	continue; */
					return -EFSCORRUPTED;
				}
				i -= lo - 2;
				continue;
			}
			++nblk;
		}
	}
	in += (vcnt << amortizedshift) - sizeof(__le32);
	m->pblk = le32_to_cpu(*(__le32 *)in) + nblk;
	return 0;
}

static int compacted_load_cluster_from_disk(struct z_erofs_maprecorder *m,
					    unsigned long lcn, bool lookahead)
{
	struct erofs_inode *const vi = m->inode;
	const unsigned int lclusterbits = vi->z_logical_clusterbits;
	const erofs_off_t ebase = round_up(iloc(vi->nid) + vi->inode_isize +
					   vi->xattr_isize, 8) +
		sizeof(struct z_erofs_map_header);
	const unsigned int totalidx = DIV_ROUND_UP(vi->i_size, EROFS_BLKSIZ);
	unsigned int compacted_4b_initial, compacted_2b;
	unsigned int amortizedshift;
	erofs_off_t pos;
	int err;

	if (lclusterbits != 12)
		return -EOPNOTSUPP;

	if (lcn >= totalidx)
		return -EINVAL;

	m->lcn = lcn;
	/* used to align to 32-byte (compacted_2b) alignment */
	compacted_4b_initial = (32 - ebase % 32) / 4;
	if (compacted_4b_initial == 32 / 4)
		compacted_4b_initial = 0;

	if (vi->z_advise & Z_EROFS_ADVISE_COMPACTED_2B)
		compacted_2b = rounddown(totalidx - compacted_4b_initial, 16);
	else
		compacted_2b = 0;

	pos = ebase;
	if (lcn < compacted_4b_initial) {
		amortizedshift = 2;
		goto out;
	}
	pos += compacted_4b_initial * 4;
	lcn -= compacted_4b_initial;

	if (lcn < compacted_2b) {
		amortizedshift = 1;
		goto out;
	}
	pos += compacted_2b * 2;
	lcn -= compacted_2b;
	amortizedshift = 2;
out:
	pos += lcn * (1 << amortizedshift);
	err = z_erofs_reload_indexes(m, erofs_blknr(pos));
	if (err)
		return err;
	return unpack_compacted_index(m, amortizedshift, erofs_blkoff(pos),
				      lookahead);
}

static int z_erofs_load_cluster_from_disk(struct z_erofs_maprecorder *m,
					  unsigned int lcn, bool lookahead)
{
	const unsigned int datamode = m->inode->datalayout;

	if (datamode == EROFS_INODE_FLAT_COMPRESSION_LEGACY)
		return legacy_load_cluster_from_disk(m, lcn);

	if (datamode == EROFS_INODE_FLAT_COMPRESSION)
		return compacted_load_cluster_from_disk(m, lcn, lookahead);

	return -EINVAL;
}

static int z_erofs_extent_lookback(struct z_erofs_maprecorder *m,
				   unsigned int lookback_distance)
{
	struct erofs_inode *const vi = m->inode;
	struct erofs_map_blocks *const map = m->map;
	const unsigned int lclusterbits = vi->z_logical_clusterbits;
	unsigned long lcn = m->lcn;
	int err;

	if (lcn < lookback_distance) {
		erofs_err("bogus lookback distance @ nid %llu",
			  (unsigned long long)vi->nid);
		DBG_BUGON(1);
		return -EFSCORRUPTED;
	}

	/* load extent head logical cluster if needed */
	lcn -= lookback_distance;
	err = z_erofs_load_cluster_from_disk(m, lcn, false);
	if (err)
		return err;

	switch (m->type) {
	case Z_EROFS_VLE_CLUSTER_TYPE_NONHEAD:
		if (!m->delta[0]) {
			erofs_err("invalid lookback distance 0 @ nid %llu",
				  (unsigned long long)vi->nid);
			DBG_BUGON(1);
			return -EFSCORRUPTED;
		}
		return z_erofs_extent_lookback(m, m->delta[0]);
	case Z_EROFS_VLE_CLUSTER_TYPE_PLAIN:
	case Z_EROFS_VLE_CLUSTER_TYPE_HEAD:
		m->headtype = m->type;
		map->m_la = (lcn << lclusterbits) | m->clusterofs;
		break;
	default:
		erofs_err("unknown type %u @ lcn %lu of nid %llu",
			  m->type, lcn, (unsigned long long)vi->nid);
		DBG_BUGON(1);
		return -EOPNOTSUPP;
	}
	return 0;
}

static int z_erofs_get_extent_compressedlen(struct z_erofs_maprecorder *m,
					    unsigned int initial_lcn)
{
	struct erofs_inode *const vi = m->inode;
	struct erofs_map_blocks *const map = m->map;
	const unsigned int lclusterbits = vi->z_logical_clusterbits;
	unsigned long lcn;
	int err;

	DBG_BUGON(m->type != Z_EROFS_VLE_CLUSTER_TYPE_PLAIN &&
		  m->type != Z_EROFS_VLE_CLUSTER_TYPE_HEAD);
	if (m->headtype == Z_EROFS_VLE_CLUSTER_TYPE_PLAIN ||
	    !(vi->z_advise & Z_EROFS_ADVISE_BIG_PCLUSTER_1)) {
		map->m_plen = 1 << lclusterbits;
		return 0;
	}

	lcn = m->lcn + 1;
	if (m->compressedlcs)
		goto out;

	err = z_erofs_load_cluster_from_disk(m, lcn, false);
	if (err)
		return err;

	/*
	 * If the 1st NONHEAD lcluster has already been handled initially w/o
	 * valid compressedlcs, which means at least it mustn't be CBLKCNT, or
	 * an internal implemenatation error is detected.
	 *
	 * The following code can also handle it properly anyway, but let's
	 * BUG_ON in the debugging mode only for developers to notice that.
	 */
	DBG_BUGON(lcn == initial_lcn &&
		  m->type == Z_EROFS_VLE_CLUSTER_TYPE_NONHEAD);

	switch (m->type) {
	case Z_EROFS_VLE_CLUSTER_TYPE_PLAIN:
	case Z_EROFS_VLE_CLUSTER_TYPE_HEAD:
		/*
		 * if the 1st NONHEAD lcluster is actually PLAIN or HEAD type
		 * rather than CBLKCNT, it's a 1 lcluster-sized pcluster.
		 */
		m->compressedlcs = 1;
		break;
	case Z_EROFS_VLE_CLUSTER_TYPE_NONHEAD:
		if (m->delta[0] != 1)
			goto err_bonus_cblkcnt;
		if (m->compressedlcs)
			break;
		/* fallthrough */
	default:
		erofs_err("cannot found CBLKCNT @ lcn %lu of nid %llu",
			  lcn, vi->nid | 0ULL);
		DBG_BUGON(1);
		return -EFSCORRUPTED;
	}
out:
	map->m_plen = m->compressedlcs << lclusterbits;
	return 0;
err_bonus_cblkcnt:
	erofs_err("bogus CBLKCNT @ lcn %lu of nid %llu",
		  lcn, vi->nid | 0ULL);
	DBG_BUGON(1);
	return -EFSCORRUPTED;
}

static int z_erofs_get_extent_decompressedlen(struct z_erofs_maprecorder *m)
{
	struct erofs_inode *const vi = m->inode;
	struct erofs_map_blocks *map = m->map;
	unsigned int lclusterbits = vi->z_logical_clusterbits;
	u64 lcn = m->lcn, headlcn = map->m_la >> lclusterbits;
	int err;

	do {
		/* handle the last EOF pcluster (no next HEAD lcluster) */
		if ((lcn << lclusterbits) >= vi->i_size) {
			map->m_llen = vi->i_size - map->m_la;
			return 0;
		}

		err = z_erofs_load_cluster_from_disk(m, lcn, true);
		if (err)
			return err;

		if (m->type == Z_EROFS_VLE_CLUSTER_TYPE_NONHEAD) {
			DBG_BUGON(!m->delta[1] &&
				  m->clusterofs != 1 << lclusterbits);
		} else if (m->type == Z_EROFS_VLE_CLUSTER_TYPE_PLAIN ||
			   m->type == Z_EROFS_VLE_CLUSTER_TYPE_HEAD) {
			/* go on until the next HEAD lcluster */
			if (lcn != headlcn)
				break;
			m->delta[1] = 1;
		} else {
			erofs_err("unknown type %u @ lcn %llu of nid %llu",
				  m->type, lcn | 0ULL,
				  (unsigned long long)vi->nid);
			DBG_BUGON(1);
			return -EOPNOTSUPP;
		}
		lcn += m->delta[1];
	} while (m->delta[1]);

	map->m_llen = (lcn << lclusterbits) + m->clusterofs - map->m_la;
	return 0;
}

int z_erofs_map_blocks_iter(struct erofs_inode *vi,
			    struct erofs_map_blocks *map,
			    int flags)
{
	struct z_erofs_maprecorder m = {
		.inode = vi,
		.map = map,
		.kaddr = map->mpage,
	};
	int err = 0;
	unsigned int lclusterbits, endoff;
	unsigned long initial_lcn;
	unsigned long long ofs, end;

	/* when trying to read beyond EOF, leave it unmapped */
	if (map->m_la >= vi->i_size) {
		map->m_llen = map->m_la + 1 - vi->i_size;
		map->m_la = vi->i_size;
		map->m_flags = 0;
		goto out;
	}

	err = z_erofs_fill_inode_lazy(vi);
	if (err)
		goto out;

	lclusterbits = vi->z_logical_clusterbits;
	ofs = map->m_la;
	initial_lcn = ofs >> lclusterbits;
	endoff = ofs & ((1 << lclusterbits) - 1);

	err = z_erofs_load_cluster_from_disk(&m, initial_lcn, false);
	if (err)
		goto out;

	map->m_flags = EROFS_MAP_MAPPED | EROFS_MAP_ENCODED;
	end = (m.lcn + 1ULL) << lclusterbits;
	switch (m.type) {
	case Z_EROFS_VLE_CLUSTER_TYPE_PLAIN:
	case Z_EROFS_VLE_CLUSTER_TYPE_HEAD:
		if (endoff >= m.clusterofs) {
			m.headtype = m.type;
			map->m_la = (m.lcn << lclusterbits) | m.clusterofs;
			break;
		}
		/* m.lcn should be >= 1 if endoff < m.clusterofs */
		if (!m.lcn) {
			erofs_err("invalid logical cluster 0 at nid %llu",
				  (unsigned long long)vi->nid);
			err = -EFSCORRUPTED;
			goto out;
		}
		end = (m.lcn << lclusterbits) | m.clusterofs;
		map->m_flags |= EROFS_MAP_FULL_MAPPED;
		m.delta[0] = 1;
		/* fallthrough */
	case Z_EROFS_VLE_CLUSTER_TYPE_NONHEAD:
		/* get the correspoinding first chunk */
		err = z_erofs_extent_lookback(&m, m.delta[0]);
		if (err)
			goto out;
		break;
	default:
		erofs_err("unknown type %u @ offset %llu of nid %llu",
			  m.type, ofs, (unsigned long long)vi->nid);
		err = -EOPNOTSUPP;
		goto out;
	}

	map->m_llen = end - map->m_la;
	map->m_pa = blknr_to_addr(m.pblk);

	err = z_erofs_get_extent_compressedlen(&m, initial_lcn);
	if (err)
		goto out;

	if (m.headtype == Z_EROFS_VLE_CLUSTER_TYPE_PLAIN)
		map->m_algorithmformat = Z_EROFS_COMPRESSION_SHIFTED;
	else
		map->m_algorithmformat = vi->z_algorithmtype[0];

	if (flags & EROFS_GET_BLOCKS_FIEMAP) {
		err = z_erofs_get_extent_decompressedlen(&m);
		if (!err)
			map->m_flags |= EROFS_MAP_FULL_MAPPED;
	}

out:
	erofs_dbg("m_la %" PRIu64 " m_pa %" PRIu64 " m_llen %" PRIu64 " m_plen %" PRIu64 " m_flags 0%o",
		  map->m_la, map->m_pa,
		  map->m_llen, map->m_plen, map->m_flags);

	DBG_BUGON(err < 0 && err != -ENOMEM);
	return err;
}
