/*
 * (C) Copyright 2005
 * 2N Telekomunikace, a.s. <www.2n.cz>
 * Ladislav Michl <michl@2n.cz>
 *
 * See file CREDITS for list of people who contributed to this
 * project.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * version 2 as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	 See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA 02111-1307 USA
 */

#ifndef _NAND_H_
#define _NAND_H_

extern void nand_init(void);

#include <linux/mtd/compat.h>
#include <linux/mtd/mtd.h>
#include <linux/mtd/nand.h>

extern int board_nand_init(struct nand_chip *nand);

typedef struct mtd_info nand_info_t;

extern int nand_curr_device;
extern nand_info_t nand_info[];

static inline int nand_read(nand_info_t *info, loff_t ofs, size_t *len, u_char *buf)
{
	return info->read(info, ofs, *len, (size_t *)len, buf);
}

static inline int nand_write(nand_info_t *info, loff_t ofs, size_t *len, u_char *buf)
{
	return info->write(info, ofs, *len, (size_t *)len, buf);
}

static inline int nand_block_isbad(nand_info_t *info, loff_t ofs)
{
	return info->block_isbad(info, ofs);
}

static inline int nand_erase(nand_info_t *info, loff_t off, size_t size)
{
	struct erase_info instr;

	instr.mtd = info;
	instr.addr = off;
	instr.len = size;
	instr.callback = 0;

	return info->erase(info, &instr);
}


/*****************************************************************************
 * declarations from nand_util.c
 ****************************************************************************/

struct nand_write_options {
	u_char *buffer;		/* memory block containing image to write */
	ulong length;		/* number of bytes to write */
	ulong offset;		/* start address in NAND */
	int quiet;		/* don't display progress messages */
	int autoplace;		/* if true use auto oob layout */
	int forcejffs2;		/* force jffs2 oob layout */
	int forceyaffs;		/* force yaffs oob layout */
	int noecc;		/* write without ecc */
	int writeoob;		/* image contains oob data */
	int pad;		/* pad to page size */
	int blockalign;		/* 1|2|4 set multiple of eraseblocks
				 * to align to */
};

typedef struct nand_write_options nand_write_options_t;
typedef struct mtd_oob_ops mtd_oob_ops_t;

struct nand_read_options {
	u_char *buffer;		/* memory block in which read image is written*/
	ulong length;		/* number of bytes to read */
	ulong offset;		/* start address in NAND */
	int quiet;		/* don't display progress messages */
	int readoob;		/* put oob data in image */
};

typedef struct nand_read_options nand_read_options_t;

struct nand_erase_options {
	loff_t length;		/* number of bytes to erase */
	loff_t offset;		/* first address in NAND to erase */
	int quiet;		/* don't display progress messages */
	int jffs2;		/* if true: format for jffs2 usage
				 * (write appropriate cleanmarker blocks) */
	int scrub;		/* if true, really clean NAND by erasing
				 * bad blocks (UNSAFE) */

	/* Don't include skipped bad blocks in size to be erased */
	int spread;
};

typedef struct nand_erase_options nand_erase_options_t;

int nand_read_skip_bad(nand_info_t *nand, loff_t offset, size_t *length,
		       u_char *buffer);

#define WITH_YAFFS_OOB	(1 << 0) /* whether write with yaffs format. This flag
				  * is a 'mode' meaning it cannot be mixed with
				  * other flags */
#define WITH_DROP_FFS	(1 << 1) /* drop trailing all-0xff pages */

int nand_write_skip_bad(nand_info_t *nand, loff_t offset, size_t *length,
			u_char *buffer, int flags);
int nand_erase_opts(nand_info_t *meminfo, const nand_erase_options_t *opts);

#define NAND_LOCK_STATUS_TIGHT	0x01
#define NAND_LOCK_STATUS_LOCK	0x02
#define NAND_LOCK_STATUS_UNLOCK 0x04

int nand_lock( nand_info_t *meminfo, int tight );
int nand_unlock( nand_info_t *meminfo, ulong start, ulong length );
int nand_get_lock_status(nand_info_t *meminfo, loff_t offset);

int nand_spl_load_image(uint32_t offs, unsigned int size, void *dst);
void nand_deselect(void);

void nand_read_buf16(struct mtd_info *mtd, uint8_t *buf, int len);
void nand_read_buf(struct mtd_info *mtd, uint8_t *buf, int len);

#ifdef CONFIG_SYS_NAND_SELECT_DEVICE
void board_nand_select_device(struct nand_chip *nand, int chip);
#endif

__attribute__((noreturn)) void nand_boot(void);

#endif

#ifdef CONFIG_ENV_OFFSET_OOB
#define ENV_OOB_MARKER 0x30425645 /*"EVB0" in little-endian -- offset is stored
				    as block number*/
#define ENV_OOB_MARKER_OLD 0x30564e45 /*"ENV0" in little-endian -- offset is
					stored as byte number */
#define ENV_OFFSET_SIZE 8
int get_nand_env_oob(nand_info_t *nand, unsigned long *result);
#endif
