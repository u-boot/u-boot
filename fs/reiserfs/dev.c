/*
 *  (C) Copyright 2003 - 2004
 *  Sysgo AG, <www.elinos.com>, Pavel Bartusek <pba@sysgo.com>
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */


#include <common.h>
#include <config.h>
#include <reiserfs.h>

#include "reiserfs_private.h"

static block_dev_desc_t *reiserfs_block_dev_desc;
static disk_partition_t *part_info;


void reiserfs_set_blk_dev(block_dev_desc_t *rbdd, disk_partition_t *info)
{
	reiserfs_block_dev_desc = rbdd;
	part_info = info;
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
	    >= part_info->size)) {
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
		if (reiserfs_block_dev_desc->block_read(reiserfs_block_dev_desc,
							part_info->start +
								sector,
							1, (void *)sec_buf)
		    != 1) {
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
	if (reiserfs_block_dev_desc->block_read(reiserfs_block_dev_desc,
						part_info->start + sector,
						block_len / SECTOR_SIZE,
						(void *)buf)
	    != block_len/SECTOR_SIZE) {
		printf (" ** reiserfs_devread() read error - block\n");
		return 0;
	}
	buf+=block_len;
	byte_len-=block_len;
	sector+= block_len/SECTOR_SIZE;

	if ( byte_len != 0 ) {
		/* read rest of data which are not in whole sector */
		if (reiserfs_block_dev_desc->block_read(reiserfs_block_dev_desc,
							part_info->start +
							    sector,
							1, (void *)sec_buf)
		    != 1) {
			printf (" ** reiserfs_devread() read error - last part\n");
			return 0;
		}
		memcpy(buf, sec_buf, byte_len);
	}

	return 1;
}
