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
 * yaffs_ramem2k.c: RAM emulation in-kernel for 2K pages (YAFFS2)
 */


const char *yaffs_ramem2k_c_version = "$Id: yaffs_ramem2k.c,v 1.3 2007/02/14 01:09:06 wookey Exp $";

#ifndef __KERNEL__
#define CONFIG_YAFFS_RAM_ENABLED
#else
#include <linux/config.h>
#endif

#ifdef CONFIG_YAFFS_RAM_ENABLED

#include "yportenv.h"

#include "yaffs_nandemul2k.h"
#include "yaffs_guts.h"
#include "yaffsinterface.h"
#include "devextras.h"
#include "yaffs_packedtags2.h"



#define EM_SIZE_IN_MEG (32)
#define PAGE_DATA_SIZE  (2048)
#define PAGE_SPARE_SIZE (64)
#define PAGES_PER_BLOCK (64)



#define EM_SIZE_IN_BYTES (EM_SIZE_IN_MEG * (1<<20))

#define PAGE_TOTAL_SIZE (PAGE_DATA_SIZE+PAGE_SPARE_SIZE)

#define BLOCK_TOTAL_SIZE (PAGES_PER_BLOCK * PAGE_TOTAL_SIZE)

#define BLOCKS_PER_MEG ((1<<20)/(PAGES_PER_BLOCK * PAGE_DATA_SIZE))


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


static void nandemul_ReallyEraseBlock(int blockNumber)
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
		return YAFFS_OK;
	}
	
	
	ned.nBlocks = nBlocks = nandemul2k_CalcNBlocks();

	
	ned.block = YMALLOC(sizeof(nandemul_Block*) * nBlocks );
	
	if(!ned.block) return YAFFS_FAIL;
	
	
	

		
	for(i=fail=0; i <nBlocks; i++)
	{
		
		nandemul_Block *blk;
		
		if(!(blk = ned.block[i] = YMALLOC(sizeof(nandemul_Block))))
		{
		 fail = 1;
		}  
		else
		{
			for(j = 0; j < PAGES_PER_BLOCK; j++)
			{
				if((blk->page[j] = YMALLOC(sizeof(nandemul_Page))) == 0)
				{
					fail = 1;
				}
			}
			nandemul_ReallyEraseBlock(i);
			ned.block[i]->damaged = 0;
			nAllocated++;
		}
	}
	
	if(fail)
	{
		//Todo thump pages
		
		for(i = 0; i < nAllocated; i++)
		{
			YFREE(ned.block[i]);
		}
		YFREE(ned.block);
		
		T(YAFFS_TRACE_ALWAYS,("Allocation failed, could only allocate %dMB of %dMB requested.\n",
		   nAllocated/64,sizeInMB));
		return 0;
	}
	
	ned.nBlocks = nBlocks;
	
	initialised = 1;
	
	return 1;
}

int nandemul2k_WriteChunkWithTagsToNAND(yaffs_Device *dev,int chunkInNAND,const __u8 *data, yaffs_ExtendedTags *tags)
{
	int blk;
	int pg;
	int i;
	
	__u8 *x;

	
	blk = chunkInNAND/PAGES_PER_BLOCK;
	pg = chunkInNAND%PAGES_PER_BLOCK;
	
	
	if(data)
	{
		x = ned.block[blk]->page[pg]->data;
		
		for(i = 0; i < PAGE_DATA_SIZE; i++)
		{
			x[i] &=data[i];
		}

		ned.block[blk]->page[pg]->empty = 0;
	}
	
	
	if(tags)
	{
		x = &ned.block[blk]->page[pg]->data[PAGE_DATA_SIZE];
		
		yaffs_PackTags2((yaffs_PackedTags2 *)x,tags);
			
	}
	
	if(tags || data)
	{
		nandemul_yield(1);
	}

	return YAFFS_OK;
}


int nandemul2k_ReadChunkWithTagsFromNAND(yaffs_Device *dev,int chunkInNAND, __u8 *data, yaffs_ExtendedTags *tags)
{
	int blk;
	int pg;
	
	__u8 *x;

	
	
	blk = chunkInNAND/PAGES_PER_BLOCK;
	pg = chunkInNAND%PAGES_PER_BLOCK;
	
	
	if(data)
	{
		memcpy(data,ned.block[blk]->page[pg]->data,PAGE_DATA_SIZE);
	}
	
	
	if(tags)
	{
		x = &ned.block[blk]->page[pg]->data[PAGE_DATA_SIZE];
		
		yaffs_UnpackTags2(tags,(yaffs_PackedTags2 *)x);
	}

	return YAFFS_OK;
}


static int nandemul2k_CheckChunkErased(yaffs_Device *dev,int chunkInNAND)
{
	int blk;
	int pg;
	int i;

	
	
	blk = chunkInNAND/PAGES_PER_BLOCK;
	pg = chunkInNAND%PAGES_PER_BLOCK;
	
	
	for(i = 0; i < PAGE_TOTAL_SIZE; i++)
	{
		if(ned.block[blk]->page[pg]->data[i] != 0xFF)
		{
			return YAFFS_FAIL;
		}
	}

	return YAFFS_OK;

}

int nandemul2k_EraseBlockInNAND(yaffs_Device *dev, int blockNumber)
{
	
	
	if(blockNumber < 0 || blockNumber >= ned.nBlocks)
	{
		T(YAFFS_TRACE_ALWAYS,("Attempt to erase non-existant block %d\n",blockNumber));
	}
	else if(ned.block[blockNumber]->damaged)
	{
		T(YAFFS_TRACE_ALWAYS,("Attempt to erase damaged block %d\n",blockNumber));
	}
	else
	{
		nandemul_ReallyEraseBlock(blockNumber);
	}
	
	return YAFFS_OK;
}

int nandemul2k_InitialiseNAND(yaffs_Device *dev)
{
	CheckInit();
	return YAFFS_OK;
}
 
int nandemul2k_MarkNANDBlockBad(struct yaffs_DeviceStruct *dev, int blockNo)
{
	
	__u8 *x;
	
	x = &ned.block[blockNo]->page[0]->data[PAGE_DATA_SIZE];
	
	memset(x,0,sizeof(yaffs_PackedTags2));
	
	
	return YAFFS_OK;
	
}

int nandemul2k_QueryNANDBlock(struct yaffs_DeviceStruct *dev, int blockNo, yaffs_BlockState *state, int *sequenceNumber)
{
	yaffs_ExtendedTags tags;
	int chunkNo;

	*sequenceNumber = 0;
	
	chunkNo = blockNo * dev->nChunksPerBlock;
	
	nandemul2k_ReadChunkWithTagsFromNAND(dev,chunkNo,NULL,&tags);
	if(tags.blockBad)
	{
		*state = YAFFS_BLOCK_STATE_DEAD;
	}
	else if(!tags.chunkUsed)
	{
		*state = YAFFS_BLOCK_STATE_EMPTY;
	}
	else if(tags.chunkUsed)
	{
		*state = YAFFS_BLOCK_STATE_NEEDS_SCANNING;
		*sequenceNumber = tags.sequenceNumber;
	}
	return YAFFS_OK;
}

int nandemul2k_GetBytesPerChunk(void) { return PAGE_DATA_SIZE;}

int nandemul2k_GetChunksPerBlock(void) { return PAGES_PER_BLOCK; }
int nandemul2k_GetNumberOfBlocks(void) {return nandemul2k_CalcNBlocks();}


#endif //YAFFS_RAM_ENABLED

