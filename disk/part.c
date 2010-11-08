/*
 * (C) Copyright 2001
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

#include <common.h>
#include <command.h>
#include <ide.h>
#include <part.h>

#undef	PART_DEBUG

#ifdef	PART_DEBUG
#define	PRINTF(fmt,args...)	printf (fmt ,##args)
#else
#define PRINTF(fmt,args...)
#endif

#if (defined(CONFIG_CMD_IDE) || \
     defined(CONFIG_CMD_MG_DISK) || \
     defined(CONFIG_CMD_SATA) || \
     defined(CONFIG_CMD_SCSI) || \
     defined(CONFIG_CMD_USB) || \
     defined(CONFIG_MMC) || \
     defined(CONFIG_SYSTEMACE) )

struct block_drvr {
	char *name;
	block_dev_desc_t* (*get_dev)(int dev);
};

static const struct block_drvr block_drvr[] = {
#if defined(CONFIG_CMD_IDE)
	{ .name = "ide", .get_dev = ide_get_dev, },
#endif
#if defined(CONFIG_CMD_SATA)
	{.name = "sata", .get_dev = sata_get_dev, },
#endif
#if defined(CONFIG_CMD_SCSI)
	{ .name = "scsi", .get_dev = scsi_get_dev, },
#endif
#if defined(CONFIG_CMD_USB) && defined(CONFIG_USB_STORAGE)
	{ .name = "usb", .get_dev = usb_stor_get_dev, },
#endif
#if defined(CONFIG_MMC)
	{ .name = "mmc", .get_dev = mmc_get_dev, },
#endif
#if defined(CONFIG_SYSTEMACE)
	{ .name = "ace", .get_dev = systemace_get_dev, },
#endif
#if defined(CONFIG_CMD_MG_DISK)
	{ .name = "mgd", .get_dev = mg_disk_get_dev, },
#endif
	{ },
};

DECLARE_GLOBAL_DATA_PTR;

block_dev_desc_t *get_dev(char* ifname, int dev)
{
	const struct block_drvr *drvr = block_drvr;
	block_dev_desc_t* (*reloc_get_dev)(int dev);

	while (drvr->name) {
		reloc_get_dev = drvr->get_dev;
#ifndef CONFIG_RELOC_FIXUP_WORKS
		reloc_get_dev += gd->reloc_off;
#endif
		if (strncmp(ifname, drvr->name, strlen(drvr->name)) == 0)
			return reloc_get_dev(dev);
		drvr++;
	}
	return NULL;
}
#else
block_dev_desc_t *get_dev(char* ifname, int dev)
{
	return NULL;
}
#endif

#if (defined(CONFIG_CMD_IDE) || \
     defined(CONFIG_CMD_MG_DISK) || \
     defined(CONFIG_CMD_SATA) || \
     defined(CONFIG_CMD_SCSI) || \
     defined(CONFIG_CMD_USB) || \
     defined(CONFIG_MMC) || \
     defined(CONFIG_SYSTEMACE) )

/* ------------------------------------------------------------------------- */
/*
 * reports device info to the user
 */

#ifdef CONFIG_LBA48
typedef uint64_t lba512_t;
#else
typedef lbaint_t lba512_t;
#endif

/*
 * Overflowless variant of (block_count * mul_by / div_by)
 * when div_by > mul_by
 */
static lba512_t lba512_muldiv (lba512_t block_count, lba512_t mul_by, lba512_t div_by)
{
	lba512_t bc_quot, bc_rem;

	/* x * m / d == x / d * m + (x % d) * m / d */
	bc_quot = block_count / div_by;
	bc_rem  = block_count - div_by * bc_quot;
	return bc_quot * mul_by + (bc_rem * mul_by) / div_by;
}

void dev_print (block_dev_desc_t *dev_desc)
{
	lba512_t lba512; /* number of blocks if 512bytes block size */

	if (dev_desc->type == DEV_TYPE_UNKNOWN) {
		puts ("not available\n");
		return;
	}

	switch (dev_desc->if_type) {
	case IF_TYPE_SCSI:
		printf ("(%d:%d) Vendor: %s Prod.: %s Rev: %s\n",
			dev_desc->target,dev_desc->lun,
			dev_desc->vendor,
			dev_desc->product,
			dev_desc->revision);
		break;
	case IF_TYPE_ATAPI:
	case IF_TYPE_IDE:
	case IF_TYPE_SATA:
		printf ("Model: %s Firm: %s Ser#: %s\n",
			dev_desc->vendor,
			dev_desc->revision,
			dev_desc->product);
		break;
	case IF_TYPE_SD:
	case IF_TYPE_MMC:
	case IF_TYPE_USB:
		printf ("Vendor: %s Rev: %s Prod: %s\n",
			dev_desc->vendor,
			dev_desc->revision,
			dev_desc->product);
		break;
	case IF_TYPE_DOC:
		puts("device type DOC\n");
		return;
	case IF_TYPE_UNKNOWN:
		puts("device type unknown\n");
		return;
	default:
		printf("Unhandled device type: %i\n", dev_desc->if_type);
		return;
	}
	puts ("            Type: ");
	if (dev_desc->removable)
		puts ("Removable ");
	switch (dev_desc->type & 0x1F) {
	case DEV_TYPE_HARDDISK:
		puts ("Hard Disk");
		break;
	case DEV_TYPE_CDROM:
		puts ("CD ROM");
		break;
	case DEV_TYPE_OPDISK:
		puts ("Optical Device");
		break;
	case DEV_TYPE_TAPE:
		puts ("Tape");
		break;
	default:
		printf ("# %02X #", dev_desc->type & 0x1F);
		break;
	}
	puts ("\n");
	if ((dev_desc->lba * dev_desc->blksz)>0L) {
		ulong mb, mb_quot, mb_rem, gb, gb_quot, gb_rem;
		lbaint_t lba;

		lba = dev_desc->lba;

		lba512 = (lba * (dev_desc->blksz/512));
		/* round to 1 digit */
		mb = lba512_muldiv(lba512, 10, 2048);	/* 2048 = (1024 * 1024) / 512 MB */

		mb_quot	= mb / 10;
		mb_rem	= mb - (10 * mb_quot);

		gb = mb / 1024;
		gb_quot	= gb / 10;
		gb_rem	= gb - (10 * gb_quot);
#ifdef CONFIG_LBA48
		if (dev_desc->lba48)
			printf ("            Supports 48-bit addressing\n");
#endif
#if defined(CONFIG_SYS_64BIT_LBA)
		printf ("            Capacity: %ld.%ld MB = %ld.%ld GB (%Ld x %ld)\n",
			mb_quot, mb_rem,
			gb_quot, gb_rem,
			lba,
			dev_desc->blksz);
#else
		printf ("            Capacity: %ld.%ld MB = %ld.%ld GB (%ld x %ld)\n",
			mb_quot, mb_rem,
			gb_quot, gb_rem,
			(ulong)lba,
			dev_desc->blksz);
#endif
	} else {
		puts ("            Capacity: not available\n");
	}
}
#endif

#if (defined(CONFIG_CMD_IDE) || \
     defined(CONFIG_CMD_MG_DISK) || \
     defined(CONFIG_CMD_SATA) || \
     defined(CONFIG_CMD_SCSI) || \
     defined(CONFIG_CMD_USB) || \
     defined(CONFIG_MMC)		|| \
     defined(CONFIG_SYSTEMACE) )

#if defined(CONFIG_MAC_PARTITION) || \
    defined(CONFIG_DOS_PARTITION) || \
    defined(CONFIG_ISO_PARTITION) || \
    defined(CONFIG_AMIGA_PARTITION) || \
    defined(CONFIG_EFI_PARTITION)

void init_part (block_dev_desc_t * dev_desc)
{
#ifdef CONFIG_ISO_PARTITION
	if (test_part_iso(dev_desc) == 0) {
		dev_desc->part_type = PART_TYPE_ISO;
		return;
	}
#endif

#ifdef CONFIG_MAC_PARTITION
	if (test_part_mac(dev_desc) == 0) {
		dev_desc->part_type = PART_TYPE_MAC;
		return;
	}
#endif

/* must be placed before DOS partition detection */
#ifdef CONFIG_EFI_PARTITION
	if (test_part_efi(dev_desc) == 0) {
		dev_desc->part_type = PART_TYPE_EFI;
		return;
	}
#endif

#ifdef CONFIG_DOS_PARTITION
	if (test_part_dos(dev_desc) == 0) {
		dev_desc->part_type = PART_TYPE_DOS;
		return;
	}
#endif

#ifdef CONFIG_AMIGA_PARTITION
	if (test_part_amiga(dev_desc) == 0) {
	    dev_desc->part_type = PART_TYPE_AMIGA;
	    return;
	}
#endif
}


int get_partition_info (block_dev_desc_t *dev_desc, int part
					, disk_partition_t *info)
{
	switch (dev_desc->part_type) {
#ifdef CONFIG_MAC_PARTITION
	case PART_TYPE_MAC:
		if (get_partition_info_mac(dev_desc,part,info) == 0) {
			PRINTF ("## Valid MAC partition found ##\n");
			return (0);
		}
		break;
#endif

#ifdef CONFIG_DOS_PARTITION
	case PART_TYPE_DOS:
		if (get_partition_info_dos(dev_desc,part,info) == 0) {
			PRINTF ("## Valid DOS partition found ##\n");
			return (0);
		}
		break;
#endif

#ifdef CONFIG_ISO_PARTITION
	case PART_TYPE_ISO:
		if (get_partition_info_iso(dev_desc,part,info) == 0) {
			PRINTF ("## Valid ISO boot partition found ##\n");
			return (0);
		}
		break;
#endif

#ifdef CONFIG_AMIGA_PARTITION
	case PART_TYPE_AMIGA:
	    if (get_partition_info_amiga(dev_desc, part, info) == 0)
	    {
		PRINTF ("## Valid Amiga partition found ##\n");
		return (0);
	    }
	    break;
#endif

#ifdef CONFIG_EFI_PARTITION
	case PART_TYPE_EFI:
		if (get_partition_info_efi(dev_desc,part,info) == 0) {
			PRINTF ("## Valid EFI partition found ##\n");
			return (0);
		}
		break;
#endif
	default:
		break;
	}
	return (-1);
}

static void print_part_header (const char *type, block_dev_desc_t * dev_desc)
{
	puts ("\nPartition Map for ");
	switch (dev_desc->if_type) {
	case IF_TYPE_IDE:
		puts ("IDE");
		break;
	case IF_TYPE_SATA:
		puts ("SATA");
		break;
	case IF_TYPE_SCSI:
		puts ("SCSI");
		break;
	case IF_TYPE_ATAPI:
		puts ("ATAPI");
		break;
	case IF_TYPE_USB:
		puts ("USB");
		break;
	case IF_TYPE_DOC:
		puts ("DOC");
		break;
	case IF_TYPE_MMC:
		puts ("MMC");
		break;
	default:
		puts ("UNKNOWN");
		break;
	}
	printf (" device %d  --   Partition Type: %s\n\n",
			dev_desc->dev, type);
}

void print_part (block_dev_desc_t * dev_desc)
{

		switch (dev_desc->part_type) {
#ifdef CONFIG_MAC_PARTITION
	case PART_TYPE_MAC:
		PRINTF ("## Testing for valid MAC partition ##\n");
		print_part_header ("MAC", dev_desc);
		print_part_mac (dev_desc);
		return;
#endif
#ifdef CONFIG_DOS_PARTITION
	case PART_TYPE_DOS:
		PRINTF ("## Testing for valid DOS partition ##\n");
		print_part_header ("DOS", dev_desc);
		print_part_dos (dev_desc);
		return;
#endif

#ifdef CONFIG_ISO_PARTITION
	case PART_TYPE_ISO:
		PRINTF ("## Testing for valid ISO Boot partition ##\n");
		print_part_header ("ISO", dev_desc);
		print_part_iso (dev_desc);
		return;
#endif

#ifdef CONFIG_AMIGA_PARTITION
	case PART_TYPE_AMIGA:
	    PRINTF ("## Testing for a valid Amiga partition ##\n");
	    print_part_header ("AMIGA", dev_desc);
	    print_part_amiga (dev_desc);
	    return;
#endif

#ifdef CONFIG_EFI_PARTITION
	case PART_TYPE_EFI:
		PRINTF ("## Testing for valid EFI partition ##\n");
		print_part_header ("EFI", dev_desc);
		print_part_efi (dev_desc);
		return;
#endif
	}
	puts ("## Unknown partition table\n");
}


#else	/* neither MAC nor DOS nor ISO nor AMIGA nor EFI partition configured */
# error neither CONFIG_MAC_PARTITION nor CONFIG_DOS_PARTITION
# error nor CONFIG_ISO_PARTITION nor CONFIG_AMIGA_PARTITION
# error nor CONFIG_EFI_PARTITION configured!
#endif

#endif
