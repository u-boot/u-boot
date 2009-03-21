/*
 * This file is part of UBIFS.
 *
 * Copyright (C) 2006-2008 Nokia Corporation.
 * Copyright (C) 2006, 2007 University of Szeged, Hungary
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License version 2 as published by
 * the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for
 * more details.
 *
 * You should have received a copy of the GNU General Public License along with
 * this program; if not, write to the Free Software Foundation, Inc., 51
 * Franklin St, Fifth Floor, Boston, MA 02110-1301 USA
 *
 * Authors: Artem Bityutskiy (Битюцкий Артём)
 *          Adrian Hunter
 *          Zoltan Sogor
 */

/*
 * This file implements UBIFS I/O subsystem which provides various I/O-related
 * helper functions (reading/writing/checking/validating nodes) and implements
 * write-buffering support. Write buffers help to save space which otherwise
 * would have been wasted for padding to the nearest minimal I/O unit boundary.
 * Instead, data first goes to the write-buffer and is flushed when the
 * buffer is full or when it is not used for some time (by timer). This is
 * similar to the mechanism is used by JFFS2.
 *
 * Write-buffers are defined by 'struct ubifs_wbuf' objects and protected by
 * mutexes defined inside these objects. Since sometimes upper-level code
 * has to lock the write-buffer (e.g. journal space reservation code), many
 * functions related to write-buffers have "nolock" suffix which means that the
 * caller has to lock the write-buffer before calling this function.
 *
 * UBIFS stores nodes at 64 bit-aligned addresses. If the node length is not
 * aligned, UBIFS starts the next node from the aligned address, and the padded
 * bytes may contain any rubbish. In other words, UBIFS does not put padding
 * bytes in those small gaps. Common headers of nodes store real node lengths,
 * not aligned lengths. Indexing nodes also store real lengths in branches.
 *
 * UBIFS uses padding when it pads to the next min. I/O unit. In this case it
 * uses padding nodes or padding bytes, if the padding node does not fit.
 *
 * All UBIFS nodes are protected by CRC checksums and UBIFS checks all nodes
 * every time they are read from the flash media.
 */

#include "ubifs.h"

/**
 * ubifs_ro_mode - switch UBIFS to read read-only mode.
 * @c: UBIFS file-system description object
 * @err: error code which is the reason of switching to R/O mode
 */
void ubifs_ro_mode(struct ubifs_info *c, int err)
{
	if (!c->ro_media) {
		c->ro_media = 1;
		c->no_chk_data_crc = 0;
		ubifs_warn("switched to read-only mode, error %d", err);
		dbg_dump_stack();
	}
}

/**
 * ubifs_check_node - check node.
 * @c: UBIFS file-system description object
 * @buf: node to check
 * @lnum: logical eraseblock number
 * @offs: offset within the logical eraseblock
 * @quiet: print no messages
 * @must_chk_crc: indicates whether to always check the CRC
 *
 * This function checks node magic number and CRC checksum. This function also
 * validates node length to prevent UBIFS from becoming crazy when an attacker
 * feeds it a file-system image with incorrect nodes. For example, too large
 * node length in the common header could cause UBIFS to read memory outside of
 * allocated buffer when checking the CRC checksum.
 *
 * This function may skip data nodes CRC checking if @c->no_chk_data_crc is
 * true, which is controlled by corresponding UBIFS mount option. However, if
 * @must_chk_crc is true, then @c->no_chk_data_crc is ignored and CRC is
 * checked. Similarly, if @c->always_chk_crc is true, @c->no_chk_data_crc is
 * ignored and CRC is checked.
 *
 * This function returns zero in case of success and %-EUCLEAN in case of bad
 * CRC or magic.
 */
int ubifs_check_node(const struct ubifs_info *c, const void *buf, int lnum,
		     int offs, int quiet, int must_chk_crc)
{
	int err = -EINVAL, type, node_len;
	uint32_t crc, node_crc, magic;
	const struct ubifs_ch *ch = buf;

	ubifs_assert(lnum >= 0 && lnum < c->leb_cnt && offs >= 0);
	ubifs_assert(!(offs & 7) && offs < c->leb_size);

	magic = le32_to_cpu(ch->magic);
	if (magic != UBIFS_NODE_MAGIC) {
		if (!quiet)
			ubifs_err("bad magic %#08x, expected %#08x",
				  magic, UBIFS_NODE_MAGIC);
		err = -EUCLEAN;
		goto out;
	}

	type = ch->node_type;
	if (type < 0 || type >= UBIFS_NODE_TYPES_CNT) {
		if (!quiet)
			ubifs_err("bad node type %d", type);
		goto out;
	}

	node_len = le32_to_cpu(ch->len);
	if (node_len + offs > c->leb_size)
		goto out_len;

	if (c->ranges[type].max_len == 0) {
		if (node_len != c->ranges[type].len)
			goto out_len;
	} else if (node_len < c->ranges[type].min_len ||
		   node_len > c->ranges[type].max_len)
		goto out_len;

	if (!must_chk_crc && type == UBIFS_DATA_NODE && !c->always_chk_crc &&
	     c->no_chk_data_crc)
		return 0;

	crc = crc32(UBIFS_CRC32_INIT, buf + 8, node_len - 8);
	node_crc = le32_to_cpu(ch->crc);
	if (crc != node_crc) {
		if (!quiet)
			ubifs_err("bad CRC: calculated %#08x, read %#08x",
				  crc, node_crc);
		err = -EUCLEAN;
		goto out;
	}

	return 0;

out_len:
	if (!quiet)
		ubifs_err("bad node length %d", node_len);
out:
	if (!quiet) {
		ubifs_err("bad node at LEB %d:%d", lnum, offs);
		dbg_dump_node(c, buf);
		dbg_dump_stack();
	}
	return err;
}

/**
 * ubifs_pad - pad flash space.
 * @c: UBIFS file-system description object
 * @buf: buffer to put padding to
 * @pad: how many bytes to pad
 *
 * The flash media obliges us to write only in chunks of %c->min_io_size and
 * when we have to write less data we add padding node to the write-buffer and
 * pad it to the next minimal I/O unit's boundary. Padding nodes help when the
 * media is being scanned. If the amount of wasted space is not enough to fit a
 * padding node which takes %UBIFS_PAD_NODE_SZ bytes, we write padding bytes
 * pattern (%UBIFS_PADDING_BYTE).
 *
 * Padding nodes are also used to fill gaps when the "commit-in-gaps" method is
 * used.
 */
void ubifs_pad(const struct ubifs_info *c, void *buf, int pad)
{
	uint32_t crc;

	ubifs_assert(pad >= 0 && !(pad & 7));

	if (pad >= UBIFS_PAD_NODE_SZ) {
		struct ubifs_ch *ch = buf;
		struct ubifs_pad_node *pad_node = buf;

		ch->magic = cpu_to_le32(UBIFS_NODE_MAGIC);
		ch->node_type = UBIFS_PAD_NODE;
		ch->group_type = UBIFS_NO_NODE_GROUP;
		ch->padding[0] = ch->padding[1] = 0;
		ch->sqnum = 0;
		ch->len = cpu_to_le32(UBIFS_PAD_NODE_SZ);
		pad -= UBIFS_PAD_NODE_SZ;
		pad_node->pad_len = cpu_to_le32(pad);
		crc = crc32(UBIFS_CRC32_INIT, buf + 8, UBIFS_PAD_NODE_SZ - 8);
		ch->crc = cpu_to_le32(crc);
		memset(buf + UBIFS_PAD_NODE_SZ, 0, pad);
	} else if (pad > 0)
		/* Too little space, padding node won't fit */
		memset(buf, UBIFS_PADDING_BYTE, pad);
}

/**
 * next_sqnum - get next sequence number.
 * @c: UBIFS file-system description object
 */
static unsigned long long next_sqnum(struct ubifs_info *c)
{
	unsigned long long sqnum;

	spin_lock(&c->cnt_lock);
	sqnum = ++c->max_sqnum;
	spin_unlock(&c->cnt_lock);

	if (unlikely(sqnum >= SQNUM_WARN_WATERMARK)) {
		if (sqnum >= SQNUM_WATERMARK) {
			ubifs_err("sequence number overflow %llu, end of life",
				  sqnum);
			ubifs_ro_mode(c, -EINVAL);
		}
		ubifs_warn("running out of sequence numbers, end of life soon");
	}

	return sqnum;
}

/**
 * ubifs_prepare_node - prepare node to be written to flash.
 * @c: UBIFS file-system description object
 * @node: the node to pad
 * @len: node length
 * @pad: if the buffer has to be padded
 *
 * This function prepares node at @node to be written to the media - it
 * calculates node CRC, fills the common header, and adds proper padding up to
 * the next minimum I/O unit if @pad is not zero.
 */
void ubifs_prepare_node(struct ubifs_info *c, void *node, int len, int pad)
{
	uint32_t crc;
	struct ubifs_ch *ch = node;
	unsigned long long sqnum = next_sqnum(c);

	ubifs_assert(len >= UBIFS_CH_SZ);

	ch->magic = cpu_to_le32(UBIFS_NODE_MAGIC);
	ch->len = cpu_to_le32(len);
	ch->group_type = UBIFS_NO_NODE_GROUP;
	ch->sqnum = cpu_to_le64(sqnum);
	ch->padding[0] = ch->padding[1] = 0;
	crc = crc32(UBIFS_CRC32_INIT, node + 8, len - 8);
	ch->crc = cpu_to_le32(crc);

	if (pad) {
		len = ALIGN(len, 8);
		pad = ALIGN(len, c->min_io_size) - len;
		ubifs_pad(c, node + len, pad);
	}
}

/**
 * ubifs_read_node - read node.
 * @c: UBIFS file-system description object
 * @buf: buffer to read to
 * @type: node type
 * @len: node length (not aligned)
 * @lnum: logical eraseblock number
 * @offs: offset within the logical eraseblock
 *
 * This function reads a node of known type and and length, checks it and
 * stores in @buf. Returns zero in case of success, %-EUCLEAN if CRC mismatched
 * and a negative error code in case of failure.
 */
int ubifs_read_node(const struct ubifs_info *c, void *buf, int type, int len,
		    int lnum, int offs)
{
	int err, l;
	struct ubifs_ch *ch = buf;

	dbg_io("LEB %d:%d, %s, length %d", lnum, offs, dbg_ntype(type), len);
	ubifs_assert(lnum >= 0 && lnum < c->leb_cnt && offs >= 0);
	ubifs_assert(len >= UBIFS_CH_SZ && offs + len <= c->leb_size);
	ubifs_assert(!(offs & 7) && offs < c->leb_size);
	ubifs_assert(type >= 0 && type < UBIFS_NODE_TYPES_CNT);

	err = ubi_read(c->ubi, lnum, buf, offs, len);
	if (err && err != -EBADMSG) {
		ubifs_err("cannot read node %d from LEB %d:%d, error %d",
			  type, lnum, offs, err);
		return err;
	}

	if (type != ch->node_type) {
		ubifs_err("bad node type (%d but expected %d)",
			  ch->node_type, type);
		goto out;
	}

	err = ubifs_check_node(c, buf, lnum, offs, 0, 0);
	if (err) {
		ubifs_err("expected node type %d", type);
		return err;
	}

	l = le32_to_cpu(ch->len);
	if (l != len) {
		ubifs_err("bad node length %d, expected %d", l, len);
		goto out;
	}

	return 0;

out:
	ubifs_err("bad node at LEB %d:%d", lnum, offs);
	dbg_dump_node(c, buf);
	dbg_dump_stack();
	return -EINVAL;
}
