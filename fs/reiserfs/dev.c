/*
 *  (C) Copyright 2003 - 2004
 *  Sysgo AG, <www.elinos.com>, Pavel Bartusek <pba@sysgo.com>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 */


#include <common.h>
#if (CONFIG_COMMANDS & CFG_CMD_REISER) || defined(CONFIG_CMD_REISER)

#include <config.h>
#include <reiserfs.h>

#include "reiserfs_private.h"

static block_dev_desc_t *reiserfs_block_dev_desc;
static disk_partition_t part_info;


int reiserfs_set_blk_dev(block_dev_desc_t *rbdd, int part)
{
	reiserfs_block_dev_desc = rbdd;

	if (part == 0) {
 		/* disk doesn't use partition table */
		part_info.start = 0;
		part_info.size = rbdd->lba;
		part_info.blksz = rbdd->blksz;
	} else {
		if (get_partition_info (reiserfs_block_dev_desc, part, &part_info)) {
			return 0;
		}
	}
	return (part_info.size);
}


int reiserfs_devread (int sector, int byte_offset, int byte_len, char *buf)
{
	char sec_buf[SECTOR_SIZE];
	unsigned block_len;
/*
	unsigned len = byte_len;
	u8 *start = buf;
*/
	/*
	*  Check partition boundaries
	*/
	if (sector < 0
	    || ((sector + ((byte_offset + byte_len - 1) >> SECTOR_BITS))
	    >= part_info.size)) {
/*		errnum = ERR_OUTSIDE_PART; */
		printf (" ** reiserfs_devread() read outside partition\n");
		return 0;
	}

	/*
	 *  Get the read to the beginning of a partition.
	 */
	sector += byte_offset >> SECTOR_BITS;
	byte_offset &= SECTOR_SIZE - 1;

#if defined(DEBUG)
	printf (" <%d, %d, %d> ", sector, byte_offset, byte_len);
#endif


	if (reiserfs_block_dev_desc == NULL)
		return 0;


	if (byte_offset != 0) {
		/* read first part which isn't aligned with start of sector */
		if (reiserfs_block_dev_desc->block_read(reiserfs_block_dev_desc->dev,
		    part_info.start+sector, 1, (unsigned long *)sec_buf) != 1) {
			printf (" ** reiserfs_devread() read error\n");
			return 0;
		}
		memcpy(buf, sec_buf+byte_offset, min(SECTOR_SIZE-byte_offset, byte_len));
		buf+=min(SECTOR_SIZE-byte_offset, byte_len);
		byte_len-=min(SECTOR_SIZE-byte_offset, byte_len);
		sector++;
	}

	/* read sector aligned part */
	block_len = byte_len & ~(SECTOR_SIZE-1);
	if (reiserfs_block_dev_desc->block_read(reiserfs_block_dev_desc->dev,
	    part_info.start+sector, block_len/SECTOR_SIZE, (unsigned long *)buf) !=
	    block_len/SECTOR_SIZE) {
		printf (" ** reiserfs_devread() read error - block\n");
		return 0;
	}
	buf+=block_len;
	byte_len-=block_len;
	sector+= block_len/SECTOR_SIZE;

	if ( byte_len != 0 ) {
		/* read rest of data which are not in whole sector */
		if (reiserfs_block_dev_desc->block_read(reiserfs_block_dev_desc->dev,
		    part_info.start+sector, 1, (unsigned long *)sec_buf) != 1) {
			printf (" ** reiserfs_devread() read error - last part\n");
			return 0;
		}
		memcpy(buf, sec_buf, byte_len);
	}

	return 1;
}

#endif /* CFG_CMD_REISERFS */
