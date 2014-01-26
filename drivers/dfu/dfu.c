/*
 * dfu.c -- DFU back-end routines
 *
 * Copyright (C) 2012 Samsung Electronics
 * author: Lukasz Majewski <l.majewski@samsung.com>
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <common.h>
#include <errno.h>
#include <malloc.h>
#include <mmc.h>
#include <fat.h>
#include <dfu.h>
#include <linux/list.h>
#include <linux/compiler.h>

static bool dfu_reset_request;
static LIST_HEAD(dfu_list);
static int dfu_alt_num;

bool dfu_reset(void)
{
	return dfu_reset_request;
}

void dfu_trigger_reset()
{
	dfu_reset_request = true;
}

static int dfu_find_alt_num(const char *s)
{
	int i = 0;

	for (; *s; s++)
		if (*s == ';')
			i++;

	return ++i;
}

int dfu_init_env_entities(char *interface, int dev)
{
	const char *str_env;
	char *env_bkp;
	int ret;

	str_env = getenv("dfu_alt_info");
	if (!str_env) {
		error("\"dfu_alt_info\" env variable not defined!\n");
		return -EINVAL;
	}

	env_bkp = strdup(str_env);
	ret = dfu_config_entities(env_bkp, interface, dev);
	if (ret) {
		error("DFU entities configuration failed!\n");
		return ret;
	}

	free(env_bkp);
	return 0;
}

static unsigned char *dfu_buf;
static unsigned long dfu_buf_size = CONFIG_SYS_DFU_DATA_BUF_SIZE;

unsigned char *dfu_free_buf(void)
{
	free(dfu_buf);
	dfu_buf = NULL;
	return dfu_buf;
}

unsigned long dfu_get_buf_size(void)
{
	return dfu_buf_size;
}

unsigned char *dfu_get_buf(void)
{
	char *s;

	if (dfu_buf != NULL)
		return dfu_buf;

	s = getenv("dfu_bufsiz");
	dfu_buf_size = s ? (unsigned long)simple_strtol(s, NULL, 16) :
			CONFIG_SYS_DFU_DATA_BUF_SIZE;

	dfu_buf = memalign(CONFIG_SYS_CACHELINE_SIZE, dfu_buf_size);
	if (dfu_buf == NULL)
		printf("%s: Could not memalign 0x%lx bytes\n",
		       __func__, dfu_buf_size);

	return dfu_buf;
}

static int dfu_write_buffer_drain(struct dfu_entity *dfu)
{
	long w_size;
	int ret;

	/* flush size? */
	w_size = dfu->i_buf - dfu->i_buf_start;
	if (w_size == 0)
		return 0;

	/* update CRC32 */
	dfu->crc = crc32(dfu->crc, dfu->i_buf_start, w_size);

	ret = dfu->write_medium(dfu, dfu->offset, dfu->i_buf_start, &w_size);
	if (ret)
		debug("%s: Write error!\n", __func__);

	/* point back */
	dfu->i_buf = dfu->i_buf_start;

	/* update offset */
	dfu->offset += w_size;

	puts("#");

	return ret;
}

int dfu_write(struct dfu_entity *dfu, void *buf, int size, int blk_seq_num)
{
	int ret = 0;
	int tret;

	debug("%s: name: %s buf: 0x%p size: 0x%x p_num: 0x%x offset: 0x%llx bufoffset: 0x%x\n",
	      __func__, dfu->name, buf, size, blk_seq_num, dfu->offset,
	      dfu->i_buf - dfu->i_buf_start);

	if (!dfu->inited) {
		/* initial state */
		dfu->crc = 0;
		dfu->offset = 0;
		dfu->bad_skip = 0;
		dfu->i_blk_seq_num = 0;
		dfu->i_buf_start = dfu_get_buf();
		if (dfu->i_buf_start == NULL)
			return -ENOMEM;
		dfu->i_buf_end = dfu_get_buf() + dfu_buf_size;
		dfu->i_buf = dfu->i_buf_start;

		dfu->inited = 1;
	}

	if (dfu->i_blk_seq_num != blk_seq_num) {
		printf("%s: Wrong sequence number! [%d] [%d]\n",
		       __func__, dfu->i_blk_seq_num, blk_seq_num);
		return -1;
	}

	/* DFU 1.1 standard says:
	 * The wBlockNum field is a block sequence number. It increments each
	 * time a block is transferred, wrapping to zero from 65,535. It is used
	 * to provide useful context to the DFU loader in the device."
	 *
	 * This means that it's a 16 bit counter that roll-overs at
	 * 0xffff -> 0x0000. By having a typical 4K transfer block
	 * we roll-over at exactly 256MB. Not very fun to debug.
	 *
	 * Handling rollover, and having an inited variable,
	 * makes things work.
	 */

	/* handle rollover */
	dfu->i_blk_seq_num = (dfu->i_blk_seq_num + 1) & 0xffff;

	/* flush buffer if overflow */
	if ((dfu->i_buf + size) > dfu->i_buf_end) {
		tret = dfu_write_buffer_drain(dfu);
		if (ret == 0)
			ret = tret;
	}

	/* we should be in buffer now (if not then size too large) */
	if ((dfu->i_buf + size) > dfu->i_buf_end) {
		error("Buffer overflow! (0x%p + 0x%x > 0x%p)\n", dfu->i_buf,
		      size, dfu->i_buf_end);
		return -1;
	}

	memcpy(dfu->i_buf, buf, size);
	dfu->i_buf += size;

	/* if end or if buffer full flush */
	if (size == 0 || (dfu->i_buf + size) > dfu->i_buf_end) {
		tret = dfu_write_buffer_drain(dfu);
		if (ret == 0)
			ret = tret;
	}

	/* end? */
	if (size == 0) {
		/* Now try and flush to the medium if needed. */
		if (dfu->flush_medium)
			ret = dfu->flush_medium(dfu);
		printf("\nDFU complete CRC32: 0x%08x\n", dfu->crc);

		/* clear everything */
		dfu_free_buf();
		dfu->crc = 0;
		dfu->offset = 0;
		dfu->i_blk_seq_num = 0;
		dfu->i_buf_start = dfu_buf;
		dfu->i_buf_end = dfu_buf;
		dfu->i_buf = dfu->i_buf_start;

		dfu->inited = 0;

	}

	return ret = 0 ? size : ret;
}

static int dfu_read_buffer_fill(struct dfu_entity *dfu, void *buf, int size)
{
	long chunk;
	int ret, readn;

	readn = 0;
	while (size > 0) {
		/* get chunk that can be read */
		chunk = min(size, dfu->b_left);
		/* consume */
		if (chunk > 0) {
			memcpy(buf, dfu->i_buf, chunk);
			dfu->crc = crc32(dfu->crc, buf, chunk);
			dfu->i_buf += chunk;
			dfu->b_left -= chunk;
			dfu->r_left -= chunk;
			size -= chunk;
			buf += chunk;
			readn += chunk;
		}

		/* all done */
		if (size > 0) {
			/* no more to read */
			if (dfu->r_left == 0)
				break;

			dfu->i_buf = dfu->i_buf_start;
			dfu->b_left = dfu->i_buf_end - dfu->i_buf_start;

			/* got to read, but buffer is empty */
			if (dfu->b_left > dfu->r_left)
				dfu->b_left = dfu->r_left;
			ret = dfu->read_medium(dfu, dfu->offset, dfu->i_buf,
					&dfu->b_left);
			if (ret != 0) {
				debug("%s: Read error!\n", __func__);
				return ret;
			}
			dfu->offset += dfu->b_left;
			dfu->r_left -= dfu->b_left;

			puts("#");
		}
	}

	return readn;
}

int dfu_read(struct dfu_entity *dfu, void *buf, int size, int blk_seq_num)
{
	int ret = 0;

	debug("%s: name: %s buf: 0x%p size: 0x%x p_num: 0x%x i_buf: 0x%p\n",
	       __func__, dfu->name, buf, size, blk_seq_num, dfu->i_buf);

	if (!dfu->inited) {
		dfu->i_buf_start = dfu_get_buf();
		if (dfu->i_buf_start == NULL)
			return -ENOMEM;

		ret = dfu->read_medium(dfu, 0, dfu->i_buf_start, &dfu->r_left);
		if (ret != 0) {
			debug("%s: failed to get r_left\n", __func__);
			return ret;
		}

		debug("%s: %s %ld [B]\n", __func__, dfu->name, dfu->r_left);

		dfu->i_blk_seq_num = 0;
		dfu->crc = 0;
		dfu->offset = 0;
		dfu->i_buf_end = dfu_get_buf() + dfu_buf_size;
		dfu->i_buf = dfu->i_buf_start;
		dfu->b_left = min(dfu_buf_size, dfu->r_left);

		dfu->bad_skip = 0;

		dfu->inited = 1;
	}

	if (dfu->i_blk_seq_num != blk_seq_num) {
		printf("%s: Wrong sequence number! [%d] [%d]\n",
		       __func__, dfu->i_blk_seq_num, blk_seq_num);
		return -1;
	}
	/* handle rollover */
	dfu->i_blk_seq_num = (dfu->i_blk_seq_num + 1) & 0xffff;

	ret = dfu_read_buffer_fill(dfu, buf, size);
	if (ret < 0) {
		printf("%s: Failed to fill buffer\n", __func__);
		return -1;
	}

	if (ret < size) {
		debug("%s: %s CRC32: 0x%x\n", __func__, dfu->name, dfu->crc);
		puts("\nUPLOAD ... done\nCtrl+C to exit ...\n");

		dfu_free_buf();
		dfu->i_blk_seq_num = 0;
		dfu->crc = 0;
		dfu->offset = 0;
		dfu->i_buf_start = dfu_buf;
		dfu->i_buf_end = dfu_buf;
		dfu->i_buf = dfu->i_buf_start;
		dfu->b_left = 0;

		dfu->bad_skip = 0;

		dfu->inited = 0;
	}

	return ret;
}

static int dfu_fill_entity(struct dfu_entity *dfu, char *s, int alt,
			   char *interface, int num)
{
	char *st;

	debug("%s: %s interface: %s num: %d\n", __func__, s, interface, num);
	st = strsep(&s, " ");
	strcpy(dfu->name, st);

	dfu->dev_num = num;
	dfu->alt = alt;

	/* Specific for mmc device */
	if (strcmp(interface, "mmc") == 0) {
		if (dfu_fill_entity_mmc(dfu, s))
			return -1;
	} else if (strcmp(interface, "nand") == 0) {
		if (dfu_fill_entity_nand(dfu, s))
			return -1;
	} else if (strcmp(interface, "ram") == 0) {
		if (dfu_fill_entity_ram(dfu, s))
			return -1;
	} else {
		printf("%s: Device %s not (yet) supported!\n",
		       __func__,  interface);
		return -1;
	}

	return 0;
}

void dfu_free_entities(void)
{
	struct dfu_entity *dfu, *p, *t = NULL;

	list_for_each_entry_safe_reverse(dfu, p, &dfu_list, list) {
		list_del(&dfu->list);
		t = dfu;
	}
	if (t)
		free(t);
	INIT_LIST_HEAD(&dfu_list);
}

int dfu_config_entities(char *env, char *interface, int num)
{
	struct dfu_entity *dfu;
	int i, ret;
	char *s;

	dfu_alt_num = dfu_find_alt_num(env);
	debug("%s: dfu_alt_num=%d\n", __func__, dfu_alt_num);

	dfu = calloc(sizeof(*dfu), dfu_alt_num);
	if (!dfu)
		return -1;
	for (i = 0; i < dfu_alt_num; i++) {

		s = strsep(&env, ";");
		ret = dfu_fill_entity(&dfu[i], s, i, interface, num);
		if (ret)
			return -1;

		list_add_tail(&dfu[i].list, &dfu_list);
	}

	return 0;
}

const char *dfu_get_dev_type(enum dfu_device_type t)
{
	const char *dev_t[] = {NULL, "eMMC", "OneNAND", "NAND", "RAM" };
	return dev_t[t];
}

const char *dfu_get_layout(enum dfu_layout l)
{
	const char *dfu_layout[] = {NULL, "RAW_ADDR", "FAT", "EXT2",
					   "EXT3", "EXT4", "RAM_ADDR" };
	return dfu_layout[l];
}

void dfu_show_entities(void)
{
	struct dfu_entity *dfu;

	puts("DFU alt settings list:\n");

	list_for_each_entry(dfu, &dfu_list, list) {
		printf("dev: %s alt: %d name: %s layout: %s\n",
		       dfu_get_dev_type(dfu->dev_type), dfu->alt,
		       dfu->name, dfu_get_layout(dfu->layout));
	}
}

int dfu_get_alt_number(void)
{
	return dfu_alt_num;
}

struct dfu_entity *dfu_get_entity(int alt)
{
	struct dfu_entity *dfu;

	list_for_each_entry(dfu, &dfu_list, list) {
		if (dfu->alt == alt)
			return dfu;
	}

	return NULL;
}

int dfu_get_alt(char *name)
{
	struct dfu_entity *dfu;

	list_for_each_entry(dfu, &dfu_list, list) {
		if (!strncmp(dfu->name, name, strlen(dfu->name)))
			return dfu->alt;
	}

	return -ENODEV;
}
