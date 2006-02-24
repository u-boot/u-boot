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

static inline int nand_read(nand_info_t *info, ulong ofs, ulong *len, u_char *buf)
{
	return info->read(info, ofs, *len, (size_t *)len, buf);
}

static inline int nand_write(nand_info_t *info, ulong ofs, ulong *len, u_char *buf)
{
	return info->write(info, ofs, *len, (size_t *)len, buf);
}

static inline int nand_block_isbad(nand_info_t *info, ulong ofs)
{
	return info->block_isbad(info, ofs);
}

static inline int nand_erase(nand_info_t *info, ulong off, ulong size)
{
	struct erase_info instr;

	instr.mtd = info;
	instr.addr = off;
	instr.len = size;
	instr.callback = 0;

	return info->erase(info, &instr);
}

#endif
