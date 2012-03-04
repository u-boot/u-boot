/*
 * (C) Copyright 2001
 * Denis Peter, MPL AG Switzerland, d.peter@mpl.ch.
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
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	 See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA 02111-1307 USA
 */

#include <common.h>
#include <command.h>
#include "part_iso.h"

#if defined(CONFIG_CMD_IDE) || \
    defined(CONFIG_CMD_SCSI) || \
    defined(CONFIG_CMD_SATA) || \
    defined(CONFIG_CMD_USB) || \
    defined(CONFIG_MMC) || \
    defined(CONFIG_SYSTEMACE)

/* #define	ISO_PART_DEBUG */

#ifdef	ISO_PART_DEBUG
#define	PRINTF(fmt,args...)	printf (fmt ,##args)
#else
#define PRINTF(fmt,args...)
#endif

/* enable this if CDs are written with the PowerPC Platform ID */
#undef CHECK_FOR_POWERPC_PLATTFORM
#define CD_SECTSIZE 2048

static unsigned char tmpbuf[CD_SECTSIZE];

/* Convert char[4] in little endian format to the host format integer
 */
static inline unsigned long le32_to_int(unsigned char *le32)
{
    return ((le32[3] << 24) +
	    (le32[2] << 16) +
	    (le32[1] << 8) +
	     le32[0]
	   );
}
/* Convert char[2] in little endian format to the host format integer
 */
static inline unsigned short le16_to_int(unsigned char *le16)
{
    return ((le16[1] << 8) +
	   le16[0]
	   );
}


/* only boot records will be listed as valid partitions */
int get_partition_info_iso_verb(block_dev_desc_t * dev_desc, int part_num, disk_partition_t * info, int verb)
{
	int i,offset,entry_num;
	unsigned short *chksumbuf;
	unsigned short chksum;
	unsigned long newblkaddr,blkaddr,lastsect,bootaddr;
	iso_boot_rec_t *pbr = (iso_boot_rec_t	*)tmpbuf; /* boot record */
	iso_pri_rec_t *ppr = (iso_pri_rec_t	*)tmpbuf;	/* primary desc */
	iso_val_entry_t *pve = (iso_val_entry_t *)tmpbuf;
	iso_init_def_entry_t *pide;

	/* the first sector (sector 0x10) must be a primary volume desc */
	blkaddr=PVD_OFFSET;
	if (dev_desc->block_read (dev_desc->dev, PVD_OFFSET, 1, (ulong *) tmpbuf) != 1)
	return (-1);
	if(ppr->desctype!=0x01) {
		if(verb)
			printf ("** First descriptor is NOT a primary desc on %d:%d **\n",
				dev_desc->dev, part_num);
		return (-1);
	}
	if(strncmp((char *)ppr->stand_ident,"CD001",5)!=0) {
		if(verb)
			printf ("** Wrong ISO Ident: %s on %d:%d **\n",
				ppr->stand_ident,dev_desc->dev, part_num);
		return (-1);
	}
	lastsect= ((ppr->firstsek_LEpathtab1_LE & 0x000000ff)<<24) +
		  ((ppr->firstsek_LEpathtab1_LE & 0x0000ff00)<< 8) +
		  ((ppr->firstsek_LEpathtab1_LE & 0x00ff0000)>> 8) +
		  ((ppr->firstsek_LEpathtab1_LE & 0xff000000)>>24) ;
	info->blksz=ppr->secsize_BE; /* assuming same block size for all entries */
	PRINTF(" Lastsect:%08lx\n",lastsect);
	for(i=blkaddr;i<lastsect;i++) {
		PRINTF("Reading block %d\n", i);
		if (dev_desc->block_read (dev_desc->dev, i, 1, (ulong *) tmpbuf) != 1)
		return (-1);
		if(ppr->desctype==0x00)
			break; /* boot entry found */
		if(ppr->desctype==0xff) {
			if(verb)
				printf ("** No valid boot catalog found on %d:%d **\n",
					dev_desc->dev, part_num);
			return (-1);
		}
	}
	/* boot entry found */
	if(strncmp(pbr->ident_str,"EL TORITO SPECIFICATION",23)!=0) {
		if(verb)
			printf ("** Wrong El Torito ident: %s on %d:%d **\n",
				pbr->ident_str,dev_desc->dev, part_num);
		return (-1);
	}
	bootaddr=le32_to_int(pbr->pointer);
	PRINTF(" Boot Entry at: %08lX\n",bootaddr);
	if (dev_desc->block_read (dev_desc->dev, bootaddr, 1, (ulong *) tmpbuf) != 1) {
		if(verb)
			printf ("** Can't read Boot Entry at %lX on %d:%d **\n",
				bootaddr,dev_desc->dev, part_num);
		return (-1);
	}
	chksum=0;
	chksumbuf = (unsigned short *)tmpbuf;
	for(i=0;i<0x10;i++)
		chksum+=((chksumbuf[i] &0xff)<<8)+((chksumbuf[i] &0xff00)>>8);
	if(chksum!=0) {
		if(verb)
			printf ("** Checksum Error in booting catalog validation entry on %d:%d **\n",
				dev_desc->dev, part_num);
		return (-1);
	}
	if((pve->key[0]!=0x55)||(pve->key[1]!=0xAA)) {
		if(verb)
			printf ("** Key 0x55 0xAA error on %d:%d **\n",
				dev_desc->dev, part_num);
		return(-1);
	}
#ifdef CHECK_FOR_POWERPC_PLATTFORM
	if(pve->platform!=0x01) {
		if(verb)
			printf ("** No PowerPC platform CD on %d:%d **\n",
				dev_desc->dev, part_num);
		return(-1);
	}
#endif
	/* the validation entry seems to be ok, now search the "partition" */
	entry_num=0;
	offset=0x20;
	sprintf ((char *)info->type, "U-Boot");
	switch(dev_desc->if_type) {
		case IF_TYPE_IDE:
		case IF_TYPE_SATA:
		case IF_TYPE_ATAPI:
			sprintf ((char *)info->name, "hd%c%d",
				'a' + dev_desc->dev, part_num);
			break;
		case IF_TYPE_SCSI:
			sprintf ((char *)info->name, "sd%c%d",
				'a' + dev_desc->dev, part_num);
			break;
		case IF_TYPE_USB:
			sprintf ((char *)info->name, "usbd%c%d",
				'a' + dev_desc->dev, part_num);
			break;
		case IF_TYPE_DOC:
			sprintf ((char *)info->name, "docd%c%d",
				'a' + dev_desc->dev, part_num);
			break;
		default:
			sprintf ((char *)info->name, "xx%c%d",
				'a' + dev_desc->dev, part_num);
			break;
	}
	/* the bootcatalog (including validation Entry) is limited to 2048Bytes
	 * (63 boot entries + validation entry) */
	 while(offset<2048) {
		pide=(iso_init_def_entry_t *)&tmpbuf[offset];
		if ((pide->boot_ind==0x88) ||
		    (pide->boot_ind==0x00)) { /* Header Id for default Sections Entries */
			if(entry_num==part_num) { /* part found */
				goto found;
			}
			entry_num++; /* count partitions Entries (boot and non bootables */
			offset+=0x20;
			continue;
		}
		if ((pide->boot_ind==0x90) ||	/* Section Header Entry */
		    (pide->boot_ind==0x91) ||	/* Section Header Entry (last) */
		    (pide->boot_ind==0x44)) {	/* Extension Indicator */
			offset+=0x20; /* skip unused entries */
		}
		else {
			if(verb)
				printf ("** Partition %d not found on device %d **\n",
					part_num,dev_desc->dev);
			return(-1);
		}
	}
	/* if we reach this point entire sector has been
	 * searched w/o succsess */
	if(verb)
		printf ("** Partition %d not found on device %d **\n",
			part_num,dev_desc->dev);
	return(-1);
found:
	if(pide->boot_ind!=0x88) {
		if(verb)
			printf ("** Partition %d is not bootable on device %d **\n",
				part_num,dev_desc->dev);
		return (-1);
	}
	switch(pide->boot_media) {
		case 0x00: /* no emulation */
			info->size=le16_to_int(pide->sec_cnt)>>2;
			break;
		case 0x01:	info->size=2400>>2; break; /* 1.2MByte Floppy */
		case 0x02:	info->size=2880>>2; break; /* 1.44MByte Floppy */
		case 0x03:	info->size=5760>>2; break; /* 2.88MByte Floppy */
		case 0x04:	info->size=2880>>2; break; /* dummy (HD Emulation) */
		default:	info->size=0; break;
	}
	newblkaddr=le32_to_int(pide->rel_block_addr);
	info->start=newblkaddr;
	PRINTF(" part %d found @ %lx size %lx\n",part_num,newblkaddr,info->size);
	return 0;
}

int get_partition_info_iso(block_dev_desc_t * dev_desc, int part_num, disk_partition_t * info)
{
	return(get_partition_info_iso_verb(dev_desc, part_num, info, 1));
}


void print_part_iso(block_dev_desc_t * dev_desc)
{
	disk_partition_t info;
	int i;
	if(get_partition_info_iso_verb(dev_desc,0,&info,0)==-1) {
		printf("** No boot partition found on device %d **\n",dev_desc->dev);
		return;
	}
	printf("Part   Start     Sect x Size Type\n");
	i=0;
	do {
		printf (" %2d %8ld %8ld %6ld %.32s\n",
			i, info.start, info.size, info.blksz, info.type);
		i++;
	} while (get_partition_info_iso_verb(dev_desc,i,&info,0)!=-1);
}

int test_part_iso (block_dev_desc_t *dev_desc)
{
	disk_partition_t info;

	return(get_partition_info_iso_verb(dev_desc,0,&info,0));
}

#endif
