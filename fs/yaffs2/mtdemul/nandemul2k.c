/*
 * YAFFS: Yet Another Flash File System. A NAND-flash specific file system.
 *
 * Copyright (C) 2002-2007 Aleph One Ltd.
 *   for Toby Churchill Ltd and Brightstar Engineering
 *
 * Created by Charles Manning <charles@aleph1.co.uk>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */

/*
 *  This version hacked for emulating 2kpage NAND for YAFFS2 testing.
 */

#include <linux/config.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/version.h>
#include <linux/slab.h>
#include <linux/init.h>
#include <linux/list.h>
#include <linux/fs.h>
#include <linux/proc_fs.h>
#include <linux/pagemap.h>
#include <linux/mtd/mtd.h>
#include <linux/interrupt.h>
#include <linux/string.h>
#if (LINUX_VERSION_CODE < KERNEL_VERSION(2,5,0))
#include <linux/locks.h>
#endif

#include <asm/uaccess.h>
#include <linux/mtd/mtd.h>
#include <linux/mtd/partitions.h>
#include <linux/mtd/nand.h>
#include "../yaffs_nandemul2k.h"

#define ALLOCATE(x) kmalloc(x,GFP_KERNEL)
#define FREE(x)     kfree(x)





#define NAND_SHIFT      (11)   // Shifter for 2k
#define PAGE_DATA_SIZE  (1 << NAND_SHIFT)
#define PAGE_SPARE_SIZE (64)
#define BLK_SHIFT	6
#define PAGES_PER_BLOCK (1 << BLK_SHIFT)	// = 64


#define EM_SIZE_IN_MEG 4
#define EM_SIZE_IN_BYTES (EM_SIZE_IN_MEG * (1<<20))

#define PAGE_TOTAL_SIZE (PAGE_DATA_SIZE+PAGE_SPARE_SIZE)

#define BLOCK_TOTAL_SIZE (PAGES_PER_BLOCK * PAGE_TOTAL_SIZE)

#define BLOCKS_PER_MEG ((1<<20)/(PAGES_PER_BLOCK * PAGE_DATA_SIZE))


static struct mtd_info nandemul2k_mtd;

typedef struct 
{
	__u8 data[PAGE_TOTAL_SIZE]; // Data + spare
	int empty;      // is this empty?
} nandemul_Page;


typedef struct
{
	nandemul_Page *page[PAGES_PER_BLOCK];
	int damaged;	
} nandemul_Block;



typedef struct
{
	nandemul_Block**block;
	int nBlocks;
} nandemul_Device;

static nandemul_Device ned;

static int sizeInMB = EM_SIZE_IN_MEG;


static void nandemul_yield(int n)
{
#ifdef __KERNEL__
	if(n > 0) schedule_timeout(n);
#endif

}


static void nandemul2k_Read(void *buffer, int page, int start, int nBytes)
{
	int pg = page%PAGES_PER_BLOCK;
	int blk = page/PAGES_PER_BLOCK;
	if(buffer && nBytes > 0)
	{
		memcpy(buffer,&ned.block[blk]->page[pg]->data[start],nBytes);
	}
	
}

static void nandemul2k_Program(const void *buffer, int page, int start, int nBytes)
{
	int pg = page%PAGES_PER_BLOCK;
	int blk = page/PAGES_PER_BLOCK;
	__u8 *p;
	__u8 *b = (__u8 *)buffer;

	p = &ned.block[blk]->page[pg]->data[start];
	
	while(buffer && nBytes>0)
	{
		*p = *p & *b;
		p++;
		b++;
		nBytes--;
	}
}

static void nandemul2k_DoErase(int blockNumber)
{
	int i;
	
	nandemul_Block *blk;
	
	if(blockNumber < 0 || blockNumber >= ned.nBlocks)
	{
		return;
	}
	
	blk = ned.block[blockNumber];
	
	for(i = 0; i < PAGES_PER_BLOCK; i++)
	{
		memset(blk->page[i],0xff,sizeof(nandemul_Page));
		blk->page[i]->empty = 1;
	}
	nandemul_yield(2);
}


static int nandemul2k_CalcNBlocks(void)
{
	return EM_SIZE_IN_MEG * BLOCKS_PER_MEG;
}



static int  CheckInit(void)
{
	static int initialised = 0;
	
	int i,j;
	
	int fail = 0;
	int nBlocks; 

	int nAllocated = 0;
	
	if(initialised) 
	{
		return 0;
	}
	
	
	ned.nBlocks = nBlocks = nandemul2k_CalcNBlocks();

	
	ned.block = ALLOCATE(sizeof(nandemul_Block*) * nBlocks );
	
	if(!ned.block) return ENOMEM;
	
	
	

		
	for(i=fail=0; i <nBlocks; i++)
	{
		
		nandemul_Block *blk;
		
		if(!(blk = ned.block[i] = ALLOCATE(sizeof(nandemul_Block))))
		{
		 fail = 1;
		}  
		else
		{
			for(j = 0; j < PAGES_PER_BLOCK; j++)
			{
				if((blk->page[j] = ALLOCATE(sizeof(nandemul_Page))) == 0)
				{
					fail = 1;
				}
			}
			nandemul2k_DoErase(i);
			ned.block[i]->damaged = 0;
			nAllocated++;
		}
	}
	
	if(fail)
	{
		//Todo thump pages
		
		for(i = 0; i < nAllocated; i++)
		{
			FREE(ned.block[i]);
		}
		FREE(ned.block);
		
		return ENOMEM;
	}
	
	ned.nBlocks = nBlocks;
	
	initialised = 1;
	
	return 1;
}



static void nandemul2k_CleanUp(void)
{
	int i,j;
	
	for(i = 0; i < ned.nBlocks; i++)
	{
		for(j = 0; j < PAGES_PER_BLOCK; j++)
		{
		   FREE(ned.block[i]->page[j]);
		}
		FREE(ned.block[i]);
		
	}
	FREE(ned.block);
	ned.block = 0;
}

int nandemul2k_GetBytesPerChunk(void) { return PAGE_DATA_SIZE;}

int nandemul2k_GetChunksPerBlock(void) { return PAGES_PER_BLOCK; }
int nandemul2k_GetNumberOfBlocks(void) {return nandemul2k_CalcNBlocks();}



static int nandemul2k_ReadId(__u8 *vendorId, __u8 *deviceId)
{
	*vendorId = 'Y'; 
	*deviceId = '2';
	
	return 1;
}


static int nandemul2k_ReadStatus(__u8 *status)
{
		*status = 0;
		return 1;
}


#ifdef CONFIG_MTD_NAND_ECC
#include <linux/mtd/nand_ecc.h>
#endif

/*
 * NAND low-level MTD interface functions
 */
static int nand_read (struct mtd_info *mtd, loff_t from, size_t len,
			size_t *retlen, u_char *buf);
static int nand_read_ecc (struct mtd_info *mtd, loff_t from, size_t len,
				size_t *retlen, u_char *buf, u_char *oob_buf, struct nand_oobinfo *dummy);
static int nand_read_oob (struct mtd_info *mtd, loff_t from, size_t len,
				size_t *retlen, u_char *buf);
static int nand_write (struct mtd_info *mtd, loff_t to, size_t len,
			size_t *retlen, const u_char *buf);
static int nand_write_ecc (struct mtd_info *mtd, loff_t to, size_t len,
				size_t *retlen, const u_char *buf,
				u_char *oob_buf, struct nand_oobinfo *dummy);
static int nand_write_oob (struct mtd_info *mtd, loff_t to, size_t len,
				size_t *retlen, const u_char *buf);
#if (LINUX_VERSION_CODE > KERNEL_VERSION(2,6,7))
static int nand_writev (struct mtd_info *mtd, const struct kvec *vecs,
				unsigned long count, loff_t to, size_t *retlen);
#else
static int nand_writev (struct mtd_info *mtd, const struct iovec *vecs,
				unsigned long count, loff_t to, size_t *retlen);
#endif
static int nand_erase (struct mtd_info *mtd, struct erase_info *instr);
static void nand_sync (struct mtd_info *mtd);



/*
 * NAND read
 */
static int nand_read (struct mtd_info *mtd, loff_t from, size_t len,
			size_t *retlen, u_char *buf)
{
	return nand_read_ecc (mtd, from, len, retlen, buf, NULL,NULL);
}


/*
 * NAND read with ECC
 */
static int nand_read_ecc (struct mtd_info *mtd, loff_t from, size_t len,
				size_t *retlen, u_char *buf, u_char *oob_buf,struct nand_oobinfo *oobsel)
{
	int 	start, page;
	int n = len;
	int nToCopy;



	/* Do not allow reads past end of device */
	if ((from + len) > mtd->size) {
		*retlen = 0;
		return -EINVAL;
	}


	/* Initialize return value */
	*retlen = 0;

	while(n > 0)
	{

		/* First we calculate the starting page */
		page = from >> NAND_SHIFT;

		/* Get raw starting column */

		start = from & (mtd->oobblock-1);

		// OK now check for the curveball where the start and end are in
		// the same page
		if((start + n) < mtd->oobblock)
		{
			nToCopy = n;
		}
		else
		{
			nToCopy =  mtd->oobblock - start;
		}

		nandemul2k_Read(buf, page, start, nToCopy);
		nandemul2k_Read(oob_buf,page,PAGE_DATA_SIZE,PAGE_SPARE_SIZE);

		n -= nToCopy;
		from += nToCopy;
		buf += nToCopy;
		if(oob_buf) oob_buf += PAGE_SPARE_SIZE;
		*retlen += nToCopy;

	}


	return 0;
}

/*
 * NAND read out-of-band
 */
static int nand_read_oob (struct mtd_info *mtd, loff_t from, size_t len,
				size_t *retlen, u_char *buf)
{
	int col, page;

	T(0,("nand_read_oob: from = 0x%08x, buf = 0x%08x, len = %i\n", (unsigned int) from, (unsigned int) buf,
		(int) len));

	/* Shift to get page */
	page = ((int) from) >> NAND_SHIFT;

	/* Mask to get column */
	col = from & 0x0f;

	/* Initialize return length value */
	*retlen = 0;

	/* Do not allow reads past end of device */
	if ((from + len) > mtd->size) {
		T(0,
			("nand_read_oob: Attempt read beyond end of device\n"));
		*retlen = 0;
		return -EINVAL;
	}

	nandemul2k_Read(buf,page,PAGE_DATA_SIZE + col,len);

	/* Return happy */
	*retlen = len;
	return 0;
}

/*
 * NAND write
 */
static int nand_write (struct mtd_info *mtd, loff_t to, size_t len,
			size_t *retlen, const u_char *buf)
{
	return nand_write_ecc (mtd, to, len, retlen, buf, NULL,NULL);
}

/*
 * NAND write with ECC
 */
static int nand_write_ecc (struct mtd_info *mtd, loff_t to, size_t len,
				size_t *retlen, const u_char *buf,
				u_char *oob_buf, struct nand_oobinfo *dummy)
{

	int 	start, page;
	int n = len;
	int nToCopy;



	/* Do not allow reads past end of device */
	if ((to + len) > mtd->size) {
		*retlen = 0;
		return -EINVAL;
	}


	/* Initialize return value */
	*retlen = 0;

	while(n > 0)
	{

		/* First we calculate the starting page */
		page = to >> NAND_SHIFT;

		/* Get raw starting column */

		start = to & (mtd->oobblock - 1);

		// OK now check for the curveball where the start and end are in
		// the same page
		if((start + n) < mtd->oobblock)
		{
			nToCopy = n;
		}
		else
		{
			nToCopy =  mtd->oobblock - start;
		}

		nandemul2k_Program(buf, page, start, nToCopy);
		nandemul2k_Program(oob_buf, page, PAGE_DATA_SIZE, PAGE_SPARE_SIZE);

		n -= nToCopy;
		to += nToCopy;
		buf += nToCopy;
		if(oob_buf) oob_buf += PAGE_SPARE_SIZE;
		*retlen += nToCopy;

	}


	return 0;
}

/*
 * NAND write out-of-band
 */
static int nand_write_oob (struct mtd_info *mtd, loff_t to, size_t len,
				size_t *retlen, const u_char *buf)
{
	int col, page;


	T(0,(
		"nand_read_oob: to = 0x%08x, len = %i\n", (unsigned int) to,
		(int) len));

	/* Shift to get page */
	page = ((int) to) >> NAND_SHIFT;

	/* Mask to get column */
	col = to & 0x0f;

	/* Initialize return length value */
	*retlen = 0;

	/* Do not allow reads past end of device */
	if ((to + len) > mtd->size) {
		T(0,(
		   "nand_read_oob: Attempt read beyond end of device\n"));
		*retlen = 0;
		return -EINVAL;
	}

	nandemul2k_Program(buf,page,512 + col,len);

	/* Return happy */
	*retlen = len;
	return 0;

}

/*
 * NAND write with iovec
 */
#if (LINUX_VERSION_CODE > KERNEL_VERSION(2,6,7))
static int nand_writev (struct mtd_info *mtd, const struct kvec *vecs,
				unsigned long count, loff_t to, size_t *retlen)
#else
static int nand_writev (struct mtd_info *mtd, const struct iovec *vecs,
				unsigned long count, loff_t to, size_t *retlen)
#endif
{
	return -EINVAL;
}

/*
 * NAND erase a block
 */
static int nand_erase (struct mtd_info *mtd, struct erase_info *instr)
{
	int i, nBlocks,block;

	T(0,(
		"nand_erase: start = 0x%08x, len = %i\n",
		(unsigned int) instr->addr, (unsigned int) instr->len));

	/* Start address must align on block boundary */
	if (instr->addr & (mtd->erasesize - 1)) {
		T(0,(
			"nand_erase: Unaligned address\n"));
		return -EINVAL;
	}

	/* Length must align on block boundary */
	if (instr->len & (mtd->erasesize - 1)) {
		T(0,(
			"nand_erase: Length not block aligned\n"));
		return -EINVAL;
	}

	/* Do not allow erase past end of device */
	if ((instr->len + instr->addr) > mtd->size) {
		T(0,(
			"nand_erase: Erase past end of device\n"));
		return -EINVAL;
	}

	nBlocks = instr->len >> (NAND_SHIFT + BLK_SHIFT);
	block = instr->addr >> (NAND_SHIFT + BLK_SHIFT);

	for(i = 0; i < nBlocks; i++)
	{
		nandemul2k_DoErase(block);
		block++;
	}

	instr->state = MTD_ERASE_DONE;  * change state to ERASE_DONE */

	instr->callback(instr);  * wake up */

	return 0;


}


static int nand_block_isbad(struct mtd_info *mtd, loff_t ofs)
{
	return 0;
}

static int nand_block_markbad(struct mtd_info *mtd, loff_t ofs)
{
	return 0;
}


/*
 * NAND sync
 */
static void nand_sync (struct mtd_info *mtd)
{
	T(0,("nand_sync: called\n"));

}

/*
 * Scan for the NAND device
 */
static int nandemul2k_scan (struct mtd_info *mtd,int nchips)
{
	mtd->oobblock = PAGE_DATA_SIZE;
	mtd->oobsize =  PAGE_SPARE_SIZE;
	mtd->erasesize = PAGE_DATA_SIZE * PAGES_PER_BLOCK;
	mtd->size = sizeInMB * 1024*1024;



	/* Fill in remaining MTD driver data */
	mtd->type = MTD_NANDFLASH;
	mtd->flags = MTD_CAP_NANDFLASH;
	mtd->owner = THIS_MODULE;
	mtd->ecctype = MTD_ECC_NONE;
	mtd->erase = nand_erase;
	mtd->point = NULL;
	mtd->unpoint = NULL;
	mtd->read = nand_read;
	mtd->write = nand_write;
	mtd->read_ecc = nand_read_ecc;
	mtd->write_ecc = nand_write_ecc;
	mtd->read_oob = nand_read_oob;
	mtd->write_oob = nand_write_oob;
	mtd->block_isbad = nand_block_isbad;
	mtd->block_markbad = nand_block_markbad;
	mtd->readv = NULL;
	mtd->writev = nand_writev;
	mtd->sync = nand_sync;
	mtd->lock = NULL;
	mtd->unlock = NULL;
	mtd->suspend = NULL;
	mtd->resume = NULL;

	mtd->name = "NANDemul2k";

	/* Return happy */
	return 0;
}

#if 0
#ifdef MODULE
MODULE_PARM(sizeInMB, "i");

__setup("sizeInMB=",sizeInMB);
#endif
#endif

/*
 * Define partitions for flash devices
 */

static struct mtd_partition nandemul2k_partition[] =
{
	{ .name		= "NANDemul partition 1",
	  .offset	= 0,
	  .size		= 0 },
};

static int nPartitions = sizeof(nandemul2k_partition)/sizeof(nandemul2k_partition[0]);

/*
 * Main initialization routine
 */
int __init nandemul2k_init (void)
{

	// Do the nand init
	
	CheckInit();

	nandemul2k_scan(&nandemul2k_mtd,1);

	// Build the partition table

	nandemul2k_partition[0].size = sizeInMB * 1024 * 1024;

	// Register the partition
	add_mtd_partitions(&nandemul2k_mtd,nandemul2k_partition,nPartitions);

	return 0;

}

module_init(nandemul2k_init);

/*
 * Clean up routine
 */
#ifdef MODULE
static void __exit nandemul2k_cleanup (void)
{

	nandemul2k_CleanUp();

	/* Unregister partitions */
	del_mtd_partitions(&nandemul2k_mtd);

	/* Unregister the device */
	del_mtd_device (&nandemul2k_mtd);

}
module_exit(nandemul2k_cleanup);
#endif

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Charles Manning <manningc@aleph1.co.uk>");
MODULE_DESCRIPTION("2k Page/128k Block NAND emulated in RAM");




