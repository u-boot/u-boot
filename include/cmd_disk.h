/*
 * (C) Copyright 2000
 * Wolfgang Denk, DENX Software Engineering, wd@denx.de.
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

/*
 * Harddisk support
 */
#ifndef	_CMD_DISK_H
#define	_CMD_DISK_H

#include <common.h>
#include <command.h>

/*
 * Type string for U-Boot bootable partitions
 */
#define BOOT_PART_TYPE	"U-Boot"	/* primary boot partition type	*/
#define BOOT_PART_COMP	"PPCBoot"	/* PPCBoot compatibility type	*/

#if 0

typedef	struct disk_partition {
	ulong	start;		/* # of first block in partition	*/
	ulong	size;		/* number of blocks in partition	*/
	ulong	blksz;		/* block size in bytes			*/
	uchar	name[32];	/* partition name			*/
	uchar	type[32];	/* string type description		*/
} disk_partition_t;

int get_partition_info     (block_dev_desc_t * dev_desc, int part, disk_partition_t *info);
#ifdef CONFIG_MAC_PARTITION
int get_partition_info_mac (block_dev_desc_t * dev_desc, int part, disk_partition_t *info);
#endif
#ifdef CONFIG_DOS_PARTITION
int get_partition_info_dos (block_dev_desc_t * dev_desc, int part, disk_partition_t *info);
#endif
#endif	/* 0 */

#endif	/* _CMD_DISK_H */
