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
 * yaffs_ramdisk.c: yaffs ram disk component
 * This provides a ram disk under yaffs.
 * NB this is not intended for NAND emulation.
 * Use this with dev->useNANDECC enabled, then ECC overheads are not required.
 */

const char *yaffs_ramdisk_c_version = "$Id: yaffs_ramdisk.c,v 1.4 2007/02/14 01:09:06 wookey Exp $";


#include "yportenv.h"

#include "yaffs_ramdisk.h"
#include "yaffs_guts.h"
#include "devextras.h"
#include "yaffs_packedtags1.h"



#define SIZE_IN_MB 2

#define BLOCK_SIZE (32 * 528)
#define BLOCKS_PER_MEG ((1024*1024)/(32 * 512))





typedef struct 
{
	__u8 data[528]; // Data + spare
} yramdisk_Page;

typedef struct
{
	yramdisk_Page page[32]; // The pages in the block
	
} yramdisk_Block;



typedef struct
{
	yramdisk_Block **block;
	int nBlocks;
} yramdisk_Device;

static yramdisk_Device ramdisk;

static int  CheckInit(yaffs_Device *dev)
{
	static int initialised = 0;
	
	int i;
	int fail = 0;
	//int nBlocks; 
	int nAllocated = 0;
	
	if(initialised) 
	{
		return YAFFS_OK;
	}

	initialised = 1;
	
	
	ramdisk.nBlocks = (SIZE_IN_MB * 1024 * 1024)/(16 * 1024);
	
	ramdisk.block = YMALLOC(sizeof(yramdisk_Block *) * ramdisk.nBlocks);
	
	if(!ramdisk.block) return 0;
	
	for(i=0; i <ramdisk.nBlocks; i++)
	{
		ramdisk.block[i] = NULL;
	}
	
	for(i=0; i <ramdisk.nBlocks && !fail; i++)
	{
		if((ramdisk.block[i] = YMALLOC(sizeof(yramdisk_Block))) == 0)
		{
			fail = 1;
		}
		else
		{
			yramdisk_EraseBlockInNAND(dev,i);
			nAllocated++;
		}
	}
	
	if(fail)
	{
		for(i = 0; i < nAllocated; i++)
		{
			YFREE(ramdisk.block[i]);
		}
		YFREE(ramdisk.block);
		
		T(YAFFS_TRACE_ALWAYS,("Allocation failed, could only allocate %dMB of %dMB requested.\n",
		   nAllocated/64,ramdisk.nBlocks * 528));
		return 0;
	}
	
	
	return 1;
}

int yramdisk_WriteChunkWithTagsToNAND(yaffs_Device *dev,int chunkInNAND,const __u8 *data, yaffs_ExtendedTags *tags)
{
	int blk;
	int pg;
	

	CheckInit(dev);
	
	blk = chunkInNAND/32;
	pg = chunkInNAND%32;
	
	
	if(data)
	{
		memcpy(ramdisk.block[blk]->page[pg].data,data,512);
	}
	
	
	if(tags)
	{
		yaffs_PackedTags1 pt;
		
		yaffs_PackTags1(&pt,tags);
		memcpy(&ramdisk.block[blk]->page[pg].data[512],&pt,sizeof(pt));
	}

	return YAFFS_OK;	

}


int yramdisk_ReadChunkWithTagsFromNAND(yaffs_Device *dev,int chunkInNAND, __u8 *data, yaffs_ExtendedTags *tags)
{
	int blk;
	int pg;

	
	CheckInit(dev);
	
	blk = chunkInNAND/32;
	pg = chunkInNAND%32;
	
	
	if(data)
	{
		memcpy(data,ramdisk.block[blk]->page[pg].data,512);
	}
	
	
	if(tags)
	{
		yaffs_PackedTags1 pt;
		
		memcpy(&pt,&ramdisk.block[blk]->page[pg].data[512],sizeof(pt));
		yaffs_UnpackTags1(tags,&pt);
		
	}

	return YAFFS_OK;
}


int yramdisk_CheckChunkErased(yaffs_Device *dev,int chunkInNAND)
{
	int blk;
	int pg;
	int i;

	
	CheckInit(dev);
	
	blk = chunkInNAND/32;
	pg = chunkInNAND%32;
	
	
	for(i = 0; i < 528; i++)
	{
		if(ramdisk.block[blk]->page[pg].data[i] != 0xFF)
		{
			return YAFFS_FAIL;
		}
	}

	return YAFFS_OK;

}

int yramdisk_EraseBlockInNAND(yaffs_Device *dev, int blockNumber)
{
	
	CheckInit(dev);
	
	if(blockNumber < 0 || blockNumber >= ramdisk.nBlocks)
	{
		T(YAFFS_TRACE_ALWAYS,("Attempt to erase non-existant block %d\n",blockNumber));
		return YAFFS_FAIL;
	}
	else
	{
		memset(ramdisk.block[blockNumber],0xFF,sizeof(yramdisk_Block));
		return YAFFS_OK;
	}
	
}

int yramdisk_InitialiseNAND(yaffs_Device *dev)
{
	//dev->useNANDECC = 1; // force on useNANDECC which gets faked. 
						 // This saves us doing ECC checks.
	
	return YAFFS_OK;
}


