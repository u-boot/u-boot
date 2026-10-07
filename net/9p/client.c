// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (C) 2026, Kuan-Wei Chiu <visitorckw@gmail.com>
 */

#define LOG_CATEGORY LOGC_NET

#include <9p.h>
#include <dm.h>
#include <dm/device.h>
#include <dm/device-internal.h>
#include <dm/uclass.h>
#include <log.h>
#include <malloc.h>
#include <asm/cache.h>
#include <asm/unaligned.h>
#include <linux/errno.h>
#include <linux/kernel.h>
#include <linux/string.h>
#include <linux/types.h>

static int p9_post_probe(struct udevice *dev)
{
	struct p9_client *c = dev_get_uclass_priv(dev);

	c->dev = dev;
	return 0;
}

UCLASS_DRIVER(p9) = {
	.name = "p9",
	.id = UCLASS_9P,
	.per_device_auto = sizeof(struct p9_client),
	.post_probe = p9_post_probe,
};

struct p9_client *p9_get_client(struct udevice *dev)
{
	return dev_get_uclass_priv(dev);
}

int p9_get_device(const char *tag, struct udevice **devp)
{
	struct udevice *dev;
	char *endp;
	ulong num;
	int ret;

	if (!tag || !tag[0] || !strcmp(tag, "-"))
		return uclass_first_device_err(UCLASS_9P, devp);

	num = simple_strtoul(tag, &endp, 10);
	if (*endp == '\0') {
		ret = uclass_get_device(UCLASS_9P, num, devp);
		if (!ret)
			return 0;
	}

	uclass_foreach_dev_probe(UCLASS_9P, dev) {
		const struct dm_p9_ops *ops = device_get_ops(dev);

		if (ops && ops->get_mount_tag) {
			const char *mtag = ops->get_mount_tag(dev);

			if (mtag && !strcmp(mtag, tag)) {
				*devp = dev;
				return 0;
			}
		}

		if (!strcmp(dev->name, tag)) {
			*devp = dev;
			return 0;
		}
	}

	return -ENODEV;
}

u32 p9_client_alloc_fid(struct p9_client *c)
{
	if (c->next_fid >= 0xFFFF0000 || c->next_fid == 0)
		c->next_fid = 2; /* fid 1 is reserved for root */
	return c->next_fid++;
}

static void p9_put_u16(u8 **p, u16 v)
{
	put_unaligned_le16(v, *p);
	*p += 2;
}

static void p9_put_u32(u8 **p, u32 v)
{
	put_unaligned_le32(v, *p);
	*p += 4;
}

static void p9_put_u64(u8 **p, u64 v)
{
	put_unaligned_le64(v, *p);
	*p += 8;
}

static void p9_put_str(u8 **p, const char *s)
{
	u16 len = strlen(s);

	put_unaligned_le16(len, *p);
	*p += 2;
	memcpy(*p, s, len);
	*p += len;
}

static int p9_request(struct p9_client *c, void *tx, int tx_len, void *rx, int rx_len)
{
	const struct dm_p9_ops *ops = device_get_ops(c->dev);
	int ret;

	if (!ops || !ops->request)
		return -ENOSYS;

	ret = ops->request(c->dev, tx, tx_len, rx, rx_len);
	if (ret < 0)
		return ret;
	if (ret < get_unaligned_le32(rx))
		return -EIO;

	return 0;
}

static int p9_check_error(struct p9_client *c, int expected_id, const char *step)
{
	struct p9_header *rx_hdr = (struct p9_header *)c->rx;
	u32 ecode;

	if (rx_hdr->id == P9_RLERROR) {
		ecode = get_unaligned_le32(c->rx + sizeof(struct p9_header));
		log_err("9P Error: %s failed (ecode=%u)\n", step, ecode);
		return -ecode;
	} else if (rx_hdr->id != expected_id) {
		log_err("9P Error: %s got id %d (expected %d)\n", step, rx_hdr->id, expected_id);
		return -EIO;
	}
	return 0;
}

int p9_client_version(struct p9_client *c)
{
	struct p9_header *hdr;
	u8 *ptr;
	u32 server_msize;
	u16 vlen;
	int ret;

	if (!c->tx) {
		c->tx = memalign(ARCH_DMA_MINALIGN, P9_MSIZE);
		if (!c->tx)
			return -ENOMEM;
	}
	if (!c->rx) {
		c->rx = memalign(ARCH_DMA_MINALIGN, P9_MSIZE);
		if (!c->rx) {
			free(c->tx);
			c->tx = NULL;
			return -ENOMEM;
		}
	}

	hdr = (struct p9_header *)c->tx;
	ptr = c->tx + sizeof(*hdr);
	p9_put_u32(&ptr, P9_MSIZE);
	p9_put_str(&ptr, "9P2000.L");

	hdr->size = ptr - c->tx;
	hdr->id = P9_TVERSION;
	hdr->tag = P9_NOTAG;

	ret = p9_request(c, c->tx, hdr->size, c->rx, P9_MSIZE);
	if (ret < 0)
		return ret;

	ret = p9_check_error(c, P9_RVERSION, "Tversion");
	if (ret < 0)
		return ret;

	ptr = c->rx + sizeof(*hdr);
	server_msize = get_unaligned_le32(ptr);
	ptr += 4;
	vlen = get_unaligned_le16(ptr);
	ptr += 2;

	if (vlen != strlen("9P2000.L") || memcmp(ptr, "9P2000.L", vlen) != 0) {
		log_err("9P: Unsupported server version: %.*s\n", vlen, ptr);
		return -EPROTONOSUPPORT;
	}

	c->msize = min((u32)P9_MSIZE, server_msize);
	return 0;
}

int p9_client_attach(struct p9_client *c, u32 fid, const char *uname, const char *aname)
{
	struct p9_header *hdr = (struct p9_header *)c->tx;
	u8 *ptr = c->tx + sizeof(*hdr);
	int ret;

	p9_put_u32(&ptr, fid);
	p9_put_u32(&ptr, P9_NOFID);
	p9_put_str(&ptr, uname);
	p9_put_str(&ptr, aname);
	p9_put_u32(&ptr, 0);

	hdr->size = ptr - c->tx;
	hdr->id = P9_TATTACH;
	hdr->tag = P9_REQ_TAG;

	ret = p9_request(c, c->tx, hdr->size, c->rx, c->msize ? c->msize : P9_MSIZE);
	if (ret < 0)
		return ret;

	return p9_check_error(c, P9_RATTACH, "Tattach");
}

int p9_client_walk(struct p9_client *c, u32 base_fid, u32 new_fid, const char *path)
{
	struct p9_header *hdr = (struct p9_header *)c->tx;
	u8 *ptr = c->tx + sizeof(*hdr);
	char path_buf[128];
	char *wnames[16];
	char *token, *p = path_buf;
	int nwname = 0, i, ret;
	u16 nwqid;

	if (strlcpy(path_buf, path, sizeof(path_buf)) >= sizeof(path_buf)) {
		log_err("9P: Path too long\n");
		return -ENAMETOOLONG;
	}

	while ((token = strsep(&p, "/")) != NULL) {
		if (*token == '\0' || strcmp(token, ".") == 0)
			continue;
		if (nwname >= 16) {
			log_err("9P: Path too deep (max 16 levels)\n");
			return -EINVAL;
		}
		wnames[nwname++] = token;
	}

	p9_put_u32(&ptr, base_fid);
	p9_put_u32(&ptr, new_fid);
	p9_put_u16(&ptr, nwname);
	for (i = 0; i < nwname; i++)
		p9_put_str(&ptr, wnames[i]);

	hdr->size = ptr - c->tx;
	hdr->id = P9_TWALK;
	hdr->tag = P9_REQ_TAG;

	ret = p9_request(c, c->tx, hdr->size, c->rx, c->msize ? c->msize : P9_MSIZE);
	if (ret < 0)
		return ret;

	ret = p9_check_error(c, P9_RWALK, "Twalk");
	if (ret < 0)
		return ret;

	ptr = c->rx + sizeof(*hdr);
	nwqid = get_unaligned_le16(ptr);
	if (nwqid != nwname)
		return -ENOENT;

	return 0;
}

int p9_client_lopen(struct p9_client *c, u32 fid, u32 flags)
{
	struct p9_header *hdr = (struct p9_header *)c->tx;
	u8 *ptr = c->tx + sizeof(*hdr);
	int ret;

	p9_put_u32(&ptr, fid);
	p9_put_u32(&ptr, flags);

	hdr->size = ptr - c->tx;
	hdr->id = P9_TLOPEN;
	hdr->tag = P9_REQ_TAG;

	ret = p9_request(c, c->tx, hdr->size, c->rx, c->msize ? c->msize : P9_MSIZE);
	if (ret < 0)
		return ret;

	return p9_check_error(c, P9_RLOPEN, "Tlopen");
}

int p9_client_clunk(struct p9_client *c, u32 fid)
{
	struct p9_header *hdr = (struct p9_header *)c->tx;
	u8 *ptr = c->tx + sizeof(*hdr);

	p9_put_u32(&ptr, fid);
	hdr->size = ptr - c->tx;
	hdr->id = P9_TCLUNK;
	hdr->tag = P9_REQ_TAG;

	return p9_request(c, c->tx, hdr->size, c->rx, c->msize ? c->msize : P9_MSIZE);
}

int p9_client_stat(struct p9_client *c, u32 fid, loff_t *size)
{
	struct p9_header *hdr = (struct p9_header *)c->tx;
	u8 *ptr = c->tx + sizeof(*hdr);
	u64 valid;
	int ret;

	p9_put_u32(&ptr, fid);
	p9_put_u64(&ptr, P9_GETATTR_SIZE);

	hdr->size = ptr - c->tx;
	hdr->id = P9_TGETATTR;
	hdr->tag = P9_REQ_TAG;

	ret = p9_request(c, c->tx, hdr->size, c->rx, c->msize ? c->msize : P9_MSIZE);
	if (ret < 0)
		return ret;

	ret = p9_check_error(c, P9_RGETATTR, "Tgetattr");
	if (ret == 0) {
		ptr = c->rx + sizeof(*hdr);
		valid = get_unaligned_le64(ptr);
		ptr += sizeof(valid);
		ptr += P9_GETATTR_SIZE_OFFSET;
		if (valid & P9_GETATTR_SIZE) {
			*size = get_unaligned_le64(ptr);
			return 0;
		}
	}
	return ret ? ret : -EIO;
}

int p9_client_read(struct p9_client *c, u32 fid, u64 offset, u32 count, void *buf, u32 *actread)
{
	struct p9_header *hdr = (struct p9_header *)c->tx;
	u8 *ptr = c->tx + sizeof(*hdr);
	u32 max_read = (c->msize ? c->msize : P9_MSIZE) - 24;
	u32 read_len = (count == 0 || count > max_read) ? max_read : count;
	u32 count_rx;
	int ret;

	p9_put_u32(&ptr, fid);
	p9_put_u64(&ptr, offset);
	p9_put_u32(&ptr, read_len);

	hdr->size = ptr - c->tx;
	hdr->id = P9_TREAD;
	hdr->tag = P9_REQ_TAG;

	ret = p9_request(c, c->tx, hdr->size, c->rx, c->msize ? c->msize : P9_MSIZE);
	if (ret < 0)
		return ret;

	ret = p9_check_error(c, P9_RREAD, "Tread");
	if (ret < 0)
		return ret;

	ptr = c->rx + sizeof(*hdr);
	count_rx = get_unaligned_le32(ptr);
	ptr += 4;
	if (count_rx > read_len) {
		log_err("9P: Tread returned count %u > requested %u\n", count_rx, read_len);
		return -EIO;
	}

	*actread = count_rx;
	memcpy(buf, ptr, count_rx);
	return 0;
}

int p9_client_readdir(struct p9_client *c, u32 fid, u64 offset, u32 count, void *buf, u32 *actread)
{
	struct p9_header *hdr = (struct p9_header *)c->tx;
	u8 *ptr = c->tx + sizeof(*hdr);
	u32 max_read = (c->msize ? c->msize : P9_MSIZE) - 24;
	u32 read_len = (count == 0 || count > max_read) ? max_read : count;
	u32 count_rx;
	int ret;

	p9_put_u32(&ptr, fid);
	p9_put_u64(&ptr, offset);
	p9_put_u32(&ptr, read_len);

	hdr->size = ptr - c->tx;
	hdr->id = P9_TREADDIR;
	hdr->tag = P9_REQ_TAG;

	ret = p9_request(c, c->tx, hdr->size, c->rx, c->msize ? c->msize : P9_MSIZE);
	if (ret < 0)
		return ret;

	ret = p9_check_error(c, P9_RREADDIR, "Treaddir");
	if (ret < 0)
		return ret;

	ptr = c->rx + sizeof(*hdr);
	count_rx = get_unaligned_le32(ptr);
	ptr += 4;
	if (count_rx > read_len) {
		log_err("9P: Treaddir returned count %u > requested %u\n", count_rx, read_len);
		return -EIO;
	}

	*actread = count_rx;
	memcpy(buf, ptr, count_rx);
	return 0;
}
