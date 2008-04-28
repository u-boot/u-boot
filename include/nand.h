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

#include <linux/mtd/compat.h>
#include <linux/mtd/mtd.h>
#include <linux/mtd/nand.h>

typedef struct mtd_info nand_info_t;

extern int nand_curr_device;
extern nand_info_t nand_info[];
extern void nand_init(void);

static inline int nand_read(nand_info_t *info, off_t ofs, size_t *len, u_char *buf)
{
	return info->read(info, ofs, *len, (size_t *)len, buf);
}

static inline int nand_write(nand_info_t *info, off_t ofs, size_t *len, u_char *buf)
{
	return info->write(info, ofs, *len, (size_t *)len, buf);
}

static inline int nand_block_isbad(nand_info_t *info, off_t ofs)
{
	return info->block_isbad(info, ofs);
}

static inline int nand_erase(nand_info_t *info, off_t off, size_t size)
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

struct nand_read_options {
	u_char *buffer;		/* memory block in which read image is written*/
	ulong length;		/* number of bytes to read */
	ulong offset;		/* start address in NAND */
	int quiet;		/* don't display progress messages */
	int readoob;		/* put oob data in image */
};

typedef struct nand_read_options nand_read_options_t;

struct nand_erase_options {
	ulong length;		/* number of bytes to erase */
	ulong offset;		/* first address in NAND to erase */
	int quiet;		/* don't display progress messages */
	int jffs2;		/* if true: format for jffs2 usage
				 * (write appropriate cleanmarker blocks) */
	int scrub;		/* if true, really clean NAND by erasing
				 * bad blocks (UNSAFE) */
};

typedef struct nand_erase_options nand_erase_options_t;

int nand_write_opts(nand_info_t *meminfo, const nand_write_options_t *opts);

int nand_read_opts(nand_info_t *meminfo, const nand_read_options_t *opts);
int nand_erase_opts(nand_info_t *meminfo, const nand_erase_options_t *opts);

#define NAND_LOCK_STATUS_TIGHT	0x01
#define NAND_LOCK_STATUS_LOCK	0x02
#define NAND_LOCK_STATUS_UNLOCK 0x04

int nand_lock( nand_info_t *meminfo, int tight );
int nand_unlock( nand_info_t *meminfo, ulong start, ulong length );
int nand_get_lock_status(nand_info_t *meminfo, ulong offset);

#ifdef CFG_NAND_SELECT_DEVICE
void board_nand_select_device(struct nand_chip *nand, int chip);
#endif

#endif
