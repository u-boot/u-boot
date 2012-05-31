/*
 * (C) Copyright 2010
 *   Renesas Solutions Corp.
 *   Nobuhiro Iwamatsu <nobuhiro.iwamatsu.yj@renesas.com>
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

#ifndef _ASM_ZIMAGE_H_
#define _ASM_ZIMAGE_H_

#define MOUNT_ROOT_RDONLY	0x000
#define RAMDISK_FLAGS		0x004
#define ORIG_ROOT_DEV		0x008
#define LOADER_TYPE			0x00c
#define INITRD_START		0x010
#define INITRD_SIZE			0x014
#define COMMAND_LINE		0x100

#define RD_PROMPT			(1<<15)
#define RD_DOLOAD			(1<<14)
#define CMD_ARG_RD_PROMPT	"prompt_ramdisk="
#define CMD_ARG_RD_DOLOAD	"load_ramdisk="

#endif
