/*
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

#include <config.h>
#include <common.h>
#include <mmc.h>
#include <asm/errno.h>
#include <asm/arch/hardware.h>
#include <part.h>
#include <fat.h>
#include "mmc_hw.h"
#include <asm/arch/spi.h>

#ifdef CONFIG_MMC

#undef MMC_DEBUG

static block_dev_desc_t mmc_dev;

/* these are filled out by a call to mmc_hw_get_parameters */
static int hw_size;		/* in kbytes */
static int hw_nr_sects;
static int hw_sect_size;	/* in bytes */

block_dev_desc_t * mmc_get_dev(int dev)
{
	return (block_dev_desc_t *)(&mmc_dev);
}

unsigned long mmc_block_read(int dev,
			     unsigned long start,
			     lbaint_t blkcnt,
			     void *buffer)
{
	unsigned long rc = 0;
	unsigned char *p = (unsigned char *)buffer;
	unsigned long i;
	unsigned long addr = start;

#ifdef MMC_DEBUG
	printf("mmc_block_read: start=%lu, blkcnt=%lu\n", start,
		 (unsigned long)blkcnt);
#endif

	for(i = 0; i < (unsigned long)blkcnt; i++) {
#ifdef MMC_DEBUG
		printf("mmc_read_sector: addr=%lu, buffer=%p\n", addr, p);
#endif
		(void)mmc_read_sector(addr, p);
		rc++;
		addr++;
		p += hw_sect_size;
	}

	return rc;
}

/*-----------------------------------------------------------------------------
 * Read hardware paramterers (sector size, size, number of sectors)
 */
static int mmc_hw_get_parameters(void)
{
	unsigned char csddata[16];
	unsigned int sizemult;
	unsigned int size;

	mmc_read_csd(csddata);
	hw_sect_size = 1<<(csddata[5] & 0x0f);
	size = ((csddata[6]&0x03)<<10)+(csddata[7]<<2)+(csddata[8]&0xc0);
	sizemult = ((csddata[10] & 0x80)>>7)+((csddata[9] & 0x03)<<1);
	hw_nr_sects = (size+1)*(1<<(sizemult+2));
	hw_size = hw_nr_sects*hw_sect_size/1024;

#ifdef MMC_DEBUG
	printf("mmc_hw_get_parameters: hw_sect_size=%d, hw_nr_sects=%d, "
		 "hw_size=%d\n", hw_sect_size, hw_nr_sects, hw_size);
#endif

	return 0;
}

int mmc_init(int verbose)
{
	int ret = -ENODEV;

	if (verbose)
		printf("mmc_init\n");

	spi_init();
	/* this meeds to be done twice */
	mmc_hw_init();
	udelay(1000);
	mmc_hw_init();

	mmc_hw_get_parameters();

	mmc_dev.if_type = IF_TYPE_MMC;
	mmc_dev.part_type = PART_TYPE_DOS;
	mmc_dev.dev = 0;
	mmc_dev.lun = 0;
	mmc_dev.type = 0;
	mmc_dev.blksz = hw_sect_size;
	mmc_dev.lba = hw_nr_sects;
	sprintf((char*)mmc_dev.vendor, "Unknown vendor");
	sprintf((char*)mmc_dev.product, "Unknown product");
	sprintf((char*)mmc_dev.revision, "N/A");
	mmc_dev.removable = 0;	/* should be true??? */
	mmc_dev.block_read = mmc_block_read;

	fat_register_device(&mmc_dev, 1);

	ret = 0;

	return ret;
}

int mmc_write(uchar * src, ulong dst, int size)
{
#ifdef MMC_DEBUG
	printf("mmc_write: src=%p, dst=%lu, size=%u\n", src, dst, size);
#endif
	/* Since mmc2info always returns 0 this function will never be called */
	return 0;
}

int mmc_read(ulong src, uchar * dst, int size)
{
#ifdef MMC_DEBUG
	printf("mmc_read: src=%lu, dst=%p, size=%u\n", src, dst, size);
#endif
	/* Since mmc2info always returns 0 this function will never be called */
	return 0;
}

int mmc2info(ulong addr)
{
	/* This function is used by cmd_cp to determine if source or destination
	 address resides on MMC-card or not. We do not support copy to and from
	 MMC-card so we always return 0. */
	return 0;
}

#endif /* CONFIG_MMC */
