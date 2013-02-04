/*
 *  (C) Copyright 2010,2011
 *  NVIDIA Corporation <www.nvidia.com>
 *
 * See file CREDITS for list of people who contributed to this
 * project.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA 02111-1307 USA
 */

#ifndef _FUSE_H_
#define _FUSE_H_

/* FUSE registers */
struct fuse_regs {
	u32 reserved0[64];		/* 0x00 - 0xFC: */
	u32 production_mode;		/* 0x100: FUSE_PRODUCTION_MODE */
	u32 reserved1[3];		/* 0x104 - 0x10c: */
	u32 sku_info;			/* 0x110 */
	u32 reserved2[13];		/* 0x114 - 0x144: */
	u32 fa;				/* 0x148: FUSE_FA */
	u32 reserved3[21];		/* 0x14C - 0x19C: */
	u32 security_mode;		/* 0x1A0: FUSE_SECURITY_MODE */
};

#endif	/* ifndef _FUSE_H_ */
