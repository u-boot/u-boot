/*
 * Copyright (c) 2004 Picture Elements, Inc.
 *    Stephen Williams (XXXXXXXXXXXXXXXX)
 *
 *    This source code is free software; you can redistribute it
 *    and/or modify it in source code form under the terms of the GNU
 *    General Public License as published by the Free Software
 *    Foundation; either version 2 of the License, or (at your option)
 *    any later version.
 *
 *    This program is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *    GNU General Public License for more details.
 *
 *    You should have received a copy of the GNU General Public License
 *    along with this program; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA
 */

/*
 * The Xilinx SystemACE chip support is activated by defining
 * CONFIG_SYSTEMACE to turn on support, and CFG_SYSTEMACE_BASE
 * to set the base address of the device. This code currently
 * assumes that the chip is connected via a byte-wide bus.
 *
 * The CONFIG_SYSTEMACE also adds to fat support the device class
 * "ace" that allows the user to execute "fatls ace 0" and the
 * like. This works by making the systemace_get_dev function
 * available to cmd_fat.c:get_dev and filling in a block device
 * description that has all the bits needed for FAT support to
 * read sectors.
 *
 * According to Xilinx technical support, before accessing the
 * SystemACE CF you need to set the following control bits:
 *      FORCECFGMODE : 1
 *      CFGMODE : 0
 *      CFGSTART : 0
 */

#include <common.h>
#include <command.h>
#include <systemace.h>
#include <part.h>
#include <asm/io.h>

#ifdef CONFIG_SYSTEMACE

/*
 * The ace_readw and writew functions read/write 16bit words, but the
 * offset value is the BYTE offset as most used in the Xilinx
 * datasheet for the SystemACE chip. The CFG_SYSTEMACE_BASE is defined
 * to be the base address for the chip, usually in the local
 * peripheral bus.
 */
#if (CFG_SYSTEMACE_WIDTH == 8)
#if !defined(__BIG_ENDIAN)
#define ace_readw(off) ((readb(CFG_SYSTEMACE_BASE+off)<<8) | \
			(readb(CFG_SYSTEMACE_BASE+off+1)))
#define ace_writew(val, off) {writeb(val>>8, CFG_SYSTEMACE_BASE+off); \
			      writeb(val, CFG_SYSTEMACE_BASE+off+1);}
#else
#define ace_readw(off) ((readb(CFG_SYSTEMACE_BASE+off)) | \
			(readb(CFG_SYSTEMACE_BASE+off+1)<<8))
#define ace_writew(val, off) {writeb(val, CFG_SYSTEMACE_BASE+off); \
			      writeb(val>>8, CFG_SYSTEMACE_BASE+off+1);}
#endif
#else
#define ace_readw(off) (in16(CFG_SYSTEMACE_BASE+off))
#define ace_writew(val, off) (out16(CFG_SYSTEMACE_BASE+off,val))
#endif

/* */

static unsigned long systemace_read(int dev, unsigned long start,
				    unsigned long blkcnt, void *buffer);

static block_dev_desc_t systemace_dev = { 0 };

static int get_cf_lock(void)
{
	int retry = 10;

	/* CONTROLREG = LOCKREG */
	unsigned val = ace_readw(0x18);
	val |= 0x0002;
	ace_writew((val & 0xffff), 0x18);

	/* Wait for MPULOCK in STATUSREG[15:0] */
	while (!(ace_readw(0x04) & 0x0002)) {

		if (retry < 0)
			return -1;

		udelay(100000);
		retry -= 1;
	}

	return 0;
}

static void release_cf_lock(void)
{
	unsigned val = ace_readw(0x18);
	val &= ~(0x0002);
	ace_writew((val & 0xffff), 0x18);
}

block_dev_desc_t *systemace_get_dev(int dev)
{
	/* The first time through this, the systemace_dev object is
	   not yet initialized. In that case, fill it in. */
	if (systemace_dev.blksz == 0) {
		systemace_dev.if_type = IF_TYPE_UNKNOWN;
		systemace_dev.dev = 0;
		systemace_dev.part_type = PART_TYPE_UNKNOWN;
		systemace_dev.type = DEV_TYPE_HARDDISK;
		systemace_dev.blksz = 512;
		systemace_dev.removable = 1;
		systemace_dev.block_read = systemace_read;

		/*
		 * Ensure the correct bus mode (8/16 bits) gets enabled
		 */
		ace_writew(CFG_SYSTEMACE_WIDTH == 8 ? 0 : 0x0001, 0);

		init_part(&systemace_dev);

	}

	return &systemace_dev;
}

/*
 * This function is called (by dereferencing the block_read pointer in
 * the dev_desc) to read blocks of data. The return value is the
 * number of blocks read. A zero return indicates an error.
 */
static unsigned long systemace_read(int dev, unsigned long start,
				    unsigned long blkcnt, void *buffer)
{
	int retry;
	unsigned blk_countdown;
	unsigned char *dp = buffer;
	unsigned val;

	if (get_cf_lock() < 0) {
		unsigned status = ace_readw(0x04);

		/* If CFDETECT is false, card is missing. */
		if (!(status & 0x0010)) {
			printf("** CompactFlash card not present. **\n");
			return 0;
		}

		printf("**** ACE locked away from me (STATUSREG=%04x)\n",
		       status);
		return 0;
	}
#ifdef DEBUG_SYSTEMACE
	printf("... systemace read %lu sectors at %lu\n", blkcnt, start);
#endif

	retry = 2000;
	for (;;) {
		val = ace_readw(0x04);

		/* If CFDETECT is false, card is missing. */
		if (!(val & 0x0010)) {
			printf("**** ACE CompactFlash not found.\n");
			release_cf_lock();
			return 0;
		}

		/* If RDYFORCMD, then we are ready to go. */
		if (val & 0x0100)
			break;

		if (retry < 0) {
			printf("**** SystemACE not ready.\n");
			release_cf_lock();
			return 0;
		}

		udelay(1000);
		retry -= 1;
	}

	/* The SystemACE can only transfer 256 sectors at a time, so
	   limit the current chunk of sectors. The blk_countdown
	   variable is the number of sectors left to transfer. */

	blk_countdown = blkcnt;
	while (blk_countdown > 0) {
		unsigned trans = blk_countdown;

		if (trans > 256)
			trans = 256;

#ifdef DEBUG_SYSTEMACE
		printf("... transfer %lu sector in a chunk\n", trans);
#endif
		/* Write LBA block address */
		ace_writew((start >> 0) & 0xffff, 0x10);
		ace_writew((start >> 16) & 0x0fff, 0x12);

		/* NOTE: in the Write Sector count below, a count of 0
		   causes a transfer of 256, so &0xff gives the right
		   value for whatever transfer count we want. */

		/* Write sector count | ReadMemCardData. */
		ace_writew((trans & 0xff) | 0x0300, 0x14);

/* 
 * For FPGA configuration via SystemACE is reset unacceptable
 * CFGDONE bit in STATUSREG is not set to 1.
 */
#ifndef SYSTEMACE_CONFIG_FPGA
		/* Reset the configruation controller */
		val = ace_readw(0x18);
		val |= 0x0080;
		ace_writew(val, 0x18);
#endif

		retry = trans * 16;
		while (retry > 0) {
			int idx;

			/* Wait for buffer to become ready. */
			while (!(ace_readw(0x04) & 0x0020)) {
				udelay(100);
			}

			/* Read 16 words of 2bytes from the sector buffer. */
			for (idx = 0; idx < 16; idx += 1) {
				unsigned short val = ace_readw(0x40);
				*dp++ = val & 0xff;
				*dp++ = (val >> 8) & 0xff;
			}

			retry -= 1;
		}

		/* Clear the configruation controller reset */
		val = ace_readw(0x18);
		val &= ~0x0080;
		ace_writew(val, 0x18);

		/* Count the blocks we transfer this time. */
		start += trans;
		blk_countdown -= trans;
	}

	release_cf_lock();

	return blkcnt;
}
#endif /* CONFIG_SYSTEMACE */
