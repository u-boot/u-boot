/*
 * dfu.h - DFU flashable area description
 *
 * Copyright (C) 2012 Samsung Electronics
 * authors: Andrzej Pietrasiewicz <andrzej.p@samsung.com>
 *	    Lukasz Majewski <l.majewski@samsung.com>
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

#ifndef __DFU_ENTITY_H_
#define __DFU_ENTITY_H_

#include <common.h>
#include <linux/list.h>
#include <mmc.h>

enum dfu_device_type {
	DFU_DEV_MMC = 1,
	DFU_DEV_ONENAND,
	DFU_DEV_NAND,
};

enum dfu_layout {
	DFU_RAW_ADDR = 1,
	DFU_FS_FAT,
	DFU_FS_EXT2,
	DFU_FS_EXT3,
	DFU_FS_EXT4,
};

struct mmc_internal_data {
	/* RAW programming */
	unsigned int lba_start;
	unsigned int lba_size;
	unsigned int lba_blk_size;

	/* FAT/EXT */
	unsigned int dev;
	unsigned int part;
};

static inline unsigned int get_mmc_blk_size(int dev)
{
	return find_mmc_device(dev)->read_bl_len;
}

#define DFU_NAME_SIZE 32
#define DFU_CMD_BUF_SIZE 128
#define DFU_DATA_BUF_SIZE (1024*1024*4) /* 4 MiB */

struct dfu_entity {
	char			name[DFU_NAME_SIZE];
	int                     alt;
	void                    *dev_private;
	int                     dev_num;
	enum dfu_device_type    dev_type;
	enum dfu_layout         layout;

	union {
		struct mmc_internal_data mmc;
	} data;

	int (*read_medium)(struct dfu_entity *dfu, void *buf, long *len);
	int (*write_medium)(struct dfu_entity *dfu, void *buf, long *len);

	struct list_head list;
};

int dfu_config_entities(char *s, char *interface, int num);
void dfu_free_entities(void);
void dfu_show_entities(void);
int dfu_get_alt_number(void);
const char *dfu_get_dev_type(enum dfu_device_type t);
const char *dfu_get_layout(enum dfu_layout l);
struct dfu_entity *dfu_get_entity(int alt);
char *dfu_extract_token(char** e, int *n);

int dfu_read(struct dfu_entity *de, void *buf, int size, int blk_seq_num);
int dfu_write(struct dfu_entity *de, void *buf, int size, int blk_seq_num);
/* Device specific */
#ifdef CONFIG_DFU_MMC
extern int dfu_fill_entity_mmc(struct dfu_entity *dfu, char *s);
#else
static inline int dfu_fill_entity_mmc(struct dfu_entity *dfu, char *s)
{
	puts("MMC support not available!\n");
	return -1;
}
#endif
#endif /* __DFU_ENTITY_H_ */
