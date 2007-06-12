/*
 * (C) Copyright 2001
 * Raymond Lo, lo@routefree.com
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
 * Support for harddisk partitions.
 *
 * To be compatible with LinuxPPC and Apple we use the standard Apple
 * SCSI disk partitioning scheme. For more information see:
 * http://developer.apple.com/techpubs/mac/Devices/Devices-126.html#MARKER-14-92
 */

#include <common.h>
#include <command.h>
#include <ide.h>
#include "part_dos.h"

#if ((CONFIG_COMMANDS & CFG_CMD_IDE)	 || defined(CONFIG_CMD_IDE) || \
     (CONFIG_COMMANDS & CFG_CMD_SCSI)	 || defined(CONFIG_CMD_SCSI) || \
     (CONFIG_COMMANDS & CFG_CMD_USB)	 || defined(CONFIG_CMD_USB) || \
     defined(CONFIG_MMC) || \
     defined(CONFIG_SYSTEMACE) ) && defined(CONFIG_DOS_PARTITION)

/* Convert char[4] in little endian format to the host format integer
 */
static inline int le32_to_int(unsigned char *le32)
{
    return ((le32[3] << 24) +
	    (le32[2] << 16) +
	    (le32[1] << 8) +
	     le32[0]
	   );
}

static inline int is_extended(int part_type)
{
    return (part_type == 0x5 ||
	    part_type == 0xf ||
	    part_type == 0x85);
}

static void print_one_part (dos_partition_t *p, int ext_part_sector, int part_num)
{
	int lba_start = ext_part_sector + le32_to_int (p->start4);
	int lba_size  = le32_to_int (p->size4);

	printf ("%5d\t\t%10d\t%10d\t%2x%s\n",
		part_num, lba_start, lba_size, p->sys_ind,
		(is_extended (p->sys_ind) ? " Extd" : ""));
}

static int test_block_type(unsigned char *buffer)
{
	if((buffer[DOS_PART_MAGIC_OFFSET + 0] != 0x55) ||
	    (buffer[DOS_PART_MAGIC_OFFSET + 1] != 0xaa) ) {
		return (-1);
	} /* no DOS Signature at all */
	if(strncmp((char *)&buffer[DOS_PBR_FSTYPE_OFFSET],"FAT",3)==0)
		return DOS_PBR; /* is PBR */
	return DOS_MBR;	    /* Is MBR */
}


int test_part_dos (block_dev_desc_t *dev_desc)
{
	unsigned char buffer[DEFAULT_SECTOR_SIZE];

	if ((dev_desc->block_read(dev_desc->dev, 0, 1, (ulong *) buffer) != 1) ||
	    (buffer[DOS_PART_MAGIC_OFFSET + 0] != 0x55) ||
	    (buffer[DOS_PART_MAGIC_OFFSET + 1] != 0xaa) ) {
		return (-1);
	}
	return (0);
}

/*  Print a partition that is relative to its Extended partition table
 */
static void print_partition_extended (block_dev_desc_t *dev_desc, int ext_part_sector, int relative,
							   int part_num)
{
	unsigned char buffer[DEFAULT_SECTOR_SIZE];
	dos_partition_t *pt;
	int i;

	if (dev_desc->block_read(dev_desc->dev, ext_part_sector, 1, (ulong *) buffer) != 1) {
		printf ("** Can't read partition table on %d:%d **\n",
			dev_desc->dev, ext_part_sector);
		return;
	}
	i=test_block_type(buffer);
	if(i==-1) {
		printf ("bad MBR sector signature 0x%02x%02x\n",
			buffer[DOS_PART_MAGIC_OFFSET],
			buffer[DOS_PART_MAGIC_OFFSET + 1]);
		return;
	}
	if(i==DOS_PBR) {
		printf ("    1\t\t         0\t%10ld\t%2x\n",
			dev_desc->lba, buffer[DOS_PBR_MEDIA_TYPE_OFFSET]);
		return;
	}
	/* Print all primary/logical partitions */
	pt = (dos_partition_t *) (buffer + DOS_PART_TBL_OFFSET);
	for (i = 0; i < 4; i++, pt++) {
		/*
		 * fdisk does not show the extended partitions that
		 * are not in the MBR
		 */

		if ((pt->sys_ind != 0) &&
		    (ext_part_sector == 0 || !is_extended (pt->sys_ind)) ) {
			print_one_part (pt, ext_part_sector, part_num);
		}

		/* Reverse engr the fdisk part# assignment rule! */
		if ((ext_part_sector == 0) ||
		    (pt->sys_ind != 0 && !is_extended (pt->sys_ind)) ) {
			part_num++;
		}
	}

	/* Follows the extended partitions */
	pt = (dos_partition_t *) (buffer + DOS_PART_TBL_OFFSET);
	for (i = 0; i < 4; i++, pt++) {
		if (is_extended (pt->sys_ind)) {
			int lba_start = le32_to_int (pt->start4) + relative;

			print_partition_extended (dev_desc, lba_start,
						  ext_part_sector == 0  ? lba_start
									: relative,
						  part_num);
		}
	}

	return;
}


/*  Print a partition that is relative to its Extended partition table
 */
static int get_partition_info_extended (block_dev_desc_t *dev_desc, int ext_part_sector,
				 int relative, int part_num,
				 int which_part, disk_partition_t *info)
{
	unsigned char buffer[DEFAULT_SECTOR_SIZE];
	dos_partition_t *pt;
	int i;

	if (dev_desc->block_read (dev_desc->dev, ext_part_sector, 1, (ulong *) buffer) != 1) {
		printf ("** Can't read partition table on %d:%d **\n",
			dev_desc->dev, ext_part_sector);
		return -1;
	}
	if (buffer[DOS_PART_MAGIC_OFFSET] != 0x55 ||
		buffer[DOS_PART_MAGIC_OFFSET + 1] != 0xaa) {
		printf ("bad MBR sector signature 0x%02x%02x\n",
			buffer[DOS_PART_MAGIC_OFFSET],
			buffer[DOS_PART_MAGIC_OFFSET + 1]);
		return -1;
	}

	/* Print all primary/logical partitions */
	pt = (dos_partition_t *) (buffer + DOS_PART_TBL_OFFSET);
	for (i = 0; i < 4; i++, pt++) {
		/*
		 * fdisk does not show the extended partitions that
		 * are not in the MBR
		 */
		if ((pt->sys_ind != 0) &&
		    (part_num == which_part) &&
		    (is_extended(pt->sys_ind) == 0)) {
			info->blksz = 512;
			info->start = ext_part_sector + le32_to_int (pt->start4);
			info->size  = le32_to_int (pt->size4);
			switch(dev_desc->if_type) {
				case IF_TYPE_IDE:
				case IF_TYPE_ATAPI:
					sprintf ((char *)info->name, "hd%c%d\n", 'a' + dev_desc->dev, part_num);
					break;
				case IF_TYPE_SCSI:
					sprintf ((char *)info->name, "sd%c%d\n", 'a' + dev_desc->dev, part_num);
					break;
				case IF_TYPE_USB:
					sprintf ((char *)info->name, "usbd%c%d\n", 'a' + dev_desc->dev, part_num);
					break;
				case IF_TYPE_DOC:
					sprintf ((char *)info->name, "docd%c%d\n", 'a' + dev_desc->dev, part_num);
					break;
				default:
					sprintf ((char *)info->name, "xx%c%d\n", 'a' + dev_desc->dev, part_num);
					break;
			}
			/* sprintf(info->type, "%d, pt->sys_ind); */
			sprintf ((char *)info->type, "U-Boot");
			return 0;
		}

		/* Reverse engr the fdisk part# assignment rule! */
		if ((ext_part_sector == 0) ||
		    (pt->sys_ind != 0 && !is_extended (pt->sys_ind)) ) {
			part_num++;
		}
	}

	/* Follows the extended partitions */
	pt = (dos_partition_t *) (buffer + DOS_PART_TBL_OFFSET);
	for (i = 0; i < 4; i++, pt++) {
		if (is_extended (pt->sys_ind)) {
			int lba_start = le32_to_int (pt->start4) + relative;

			return get_partition_info_extended (dev_desc, lba_start,
				 ext_part_sector == 0 ? lba_start : relative,
				 part_num, which_part, info);
		}
	}
	return -1;
}

void print_part_dos (block_dev_desc_t *dev_desc)
{
	printf ("Partition     Start Sector     Num Sectors     Type\n");
	print_partition_extended (dev_desc, 0, 0, 1);
}

int get_partition_info_dos (block_dev_desc_t *dev_desc, int part, disk_partition_t * info)
{
	return get_partition_info_extended (dev_desc, 0, 0, 1, part, info);
}


#endif	/* (CONFIG_COMMANDS & CFG_CMD_IDE) && CONFIG_DOS_PARTITION */
