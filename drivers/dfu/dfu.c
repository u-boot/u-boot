/*
 * dfu.c -- DFU back-end routines
 *
 * Copyright (C) 2012 Samsung Electronics
 * author: Lukasz Majewski <l.majewski@samsung.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#include <common.h>
#include <malloc.h>
#include <mmc.h>
#include <fat.h>
#include <dfu.h>
#include <linux/list.h>
#include <linux/compiler.h>

static LIST_HEAD(dfu_list);
static int dfu_alt_num;

static int dfu_find_alt_num(const char *s)
{
	int i = 0;

	for (; *s; s++)
		if (*s == ';')
			i++;

	return ++i;
}

static unsigned char __aligned(CONFIG_SYS_CACHELINE_SIZE)
				     dfu_buf[DFU_DATA_BUF_SIZE];

int dfu_write(struct dfu_entity *dfu, void *buf, int size, int blk_seq_num)
{
	static unsigned char *i_buf;
	static int i_blk_seq_num;
	long w_size = 0;
	int ret = 0;

	debug("%s: name: %s buf: 0x%p size: 0x%x p_num: 0x%x i_buf: 0x%p\n",
	       __func__, dfu->name, buf, size, blk_seq_num, i_buf);

	if (blk_seq_num == 0) {
		i_buf = dfu_buf;
		i_blk_seq_num = 0;
	}

	if (i_blk_seq_num++ != blk_seq_num) {
		printf("%s: Wrong sequence number! [%d] [%d]\n",
		       __func__, i_blk_seq_num, blk_seq_num);
		return -1;
	}

	memcpy(i_buf, buf, size);
	i_buf += size;

	if (size == 0) {
		/* Integrity check (if needed) */
		debug("%s: %s %d [B] CRC32: 0x%x\n", __func__, dfu->name,
		       i_buf - dfu_buf, crc32(0, dfu_buf, i_buf - dfu_buf));

		w_size = i_buf - dfu_buf;
		ret = dfu->write_medium(dfu, dfu_buf, &w_size);
		if (ret)
			debug("%s: Write error!\n", __func__);

		i_blk_seq_num = 0;
		i_buf = NULL;
		return ret;
	}

	return ret;
}

int dfu_read(struct dfu_entity *dfu, void *buf, int size, int blk_seq_num)
{
	static unsigned char *i_buf;
	static int i_blk_seq_num;
	static long r_size;
	static u32 crc;
	int ret = 0;

	debug("%s: name: %s buf: 0x%p size: 0x%x p_num: 0x%x i_buf: 0x%p\n",
	       __func__, dfu->name, buf, size, blk_seq_num, i_buf);

	if (blk_seq_num == 0) {
		i_buf = dfu_buf;
		ret = dfu->read_medium(dfu, i_buf, &r_size);
		debug("%s: %s %ld [B]\n", __func__, dfu->name, r_size);
		i_blk_seq_num = 0;
		/* Integrity check (if needed) */
		crc = crc32(0, dfu_buf, r_size);
	}

	if (i_blk_seq_num++ != blk_seq_num) {
		printf("%s: Wrong sequence number! [%d] [%d]\n",
		       __func__, i_blk_seq_num, blk_seq_num);
		return -1;
	}

	if (r_size >= size) {
		memcpy(buf, i_buf, size);
		i_buf += size;
		r_size -= size;
		return size;
	} else {
		memcpy(buf, i_buf, r_size);
		i_buf += r_size;
		debug("%s: %s CRC32: 0x%x\n", __func__, dfu->name, crc);
		puts("UPLOAD ... done\nCtrl+C to exit ...\n");

		i_buf = NULL;
		i_blk_seq_num = 0;
		crc = 0;
		return r_size;
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
	const char *dev_t[] = {NULL, "eMMC", "OneNAND", "NAND" };
	return dev_t[t];
}

const char *dfu_get_layout(enum dfu_layout l)
{
	const char *dfu_layout[] = {NULL, "RAW_ADDR", "FAT", "EXT2",
					   "EXT3", "EXT4" };
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
