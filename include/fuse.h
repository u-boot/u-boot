/*
 * (C) Copyright 2009-2013 ADVANSEE
 * Benoît Thébaudeau <benoit.thebaudeau@advansee.com>
 *
 * Based on the mpc512x iim code:
 * Copyright 2008 Silicon Turnkey Express, Inc.
 * Martha Marx <mmarx@silicontkx.com>
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

/*
 * Read/Sense/Program/Override interface:
 *   bank:    Fuse bank
 *   word:    Fuse word within the bank
 *   val:     Value to read/write
 *
 *   Returns: 0 on success, not 0 on failure
 */
int fuse_read(u32 bank, u32 word, u32 *val);
int fuse_sense(u32 bank, u32 word, u32 *val);
int fuse_prog(u32 bank, u32 word, u32 val);
int fuse_override(u32 bank, u32 word, u32 val);

#endif	/* _FUSE_H_ */
