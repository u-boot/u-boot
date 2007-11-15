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
 * This provides a YAFFS nand emulation on a file.
 * This is only intended as test code to test persistence etc.
 */

const char *yaffs_flashif_c_version = "$Id: yaffs_fileem.c,v 1.3 2007/02/14 01:09:06 wookey Exp $";


#include "yportenv.h"

#include "yaffs_flashif.h"
#include "yaffs_guts.h"

#include "devextras.h"

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h> 



#define SIZE_IN_MB 16

#define BLOCK_SIZE (32 * 528)
#define BLOCKS_PER_MEG ((1024*1024)/(32 * 512))



typedef struct 
{
	__u8 data[528]; // Data + spare
} yflash_Page;

typedef struct
{
	yflash_Page page[32]; // The pages in the block
	
} yflash_Block;



typedef struct
{
	int handle;
	int nBlocks;
} yflash_Device;

static yflash_Device filedisk;

static int  CheckInit(yaffs_Device *dev)
{
	static int initialised = 0;
	
	int i;

	
	int fSize;
	int written;
	
	yflash_Page p;
	
	if(initialised) 
	{
		return YAFFS_OK;
	}

	initialised = 1;
	
	
	filedisk.nBlocks = (SIZE_IN_MB * 1024 * 1024)/(16 * 1024);
	
	filedisk.handle = open("yaffsemfile", O_RDWR | O_CREAT, S_IREAD | S_IWRITE);
	
	if(filedisk.handle < 0)
	{
		perror("Failed to open yaffs emulation file");
		return YAFFS_FAIL;
	}
	
	
	fSize = lseek(filedisk.handle,0,SEEK_END);
	
	if(fSize < SIZE_IN_MB * 1024 * 1024)
	{
		printf("Creating yaffs emulation file\n");
		
		lseek(filedisk.handle,0,SEEK_SET);
		
		memset(&p,0xff,sizeof(yflash_Page));
		
		for(i = 0; i < SIZE_IN_MB * 1024 * 1024; i+= 512)
		{
			written = write(filedisk.handle,&p,sizeof(yflash_Page));
			
			if(written != sizeof(yflash_Page))
			{
				printf("Write failed\n");
				return YAFFS_FAIL;
			}
		}		
	}
	
	return 1;
}

int yflash_WriteChunkToNAND(yaffs_Device *dev,int chunkInNAND,const __u8 *data, const yaffs_Spare *spare)
{
	int written;

	CheckInit(dev);
	
	
	
	if(data)
	{
		lseek(filedisk.handle,chunkInNAND * 528,SEEK_SET);
		written = write(filedisk.handle,data,512);
		
		if(written != 512) return YAFFS_FAIL;
	}
	
	if(spare)
	{
		lseek(filedisk.handle,chunkInNAND * 528 + 512,SEEK_SET);
		written = write(filedisk.handle,spare,16);
		
		if(written != 16) return YAFFS_FAIL;
	}
	

	return YAFFS_OK;	

}


int yflash_ReadChunkFromNAND(yaffs_Device *dev,int chunkInNAND, __u8 *data, yaffs_Spare *spare)
{
	int nread;

	CheckInit(dev);
	
	
	
	if(data)
	{
		lseek(filedisk.handle,chunkInNAND * 528,SEEK_SET);
		nread = read(filedisk.handle,data,512);
		
		if(nread != 512) return YAFFS_FAIL;
	}
	
	if(spare)
	{
		lseek(filedisk.handle,chunkInNAND * 528 + 512,SEEK_SET);
		nread= read(filedisk.handle,spare,16);
		
		if(nread != 16) return YAFFS_FAIL;
	}
	

	return YAFFS_OK;	

}


int yflash_EraseBlockInNAND(yaffs_Device *dev, int blockNumber)
{

	int i;
		
	CheckInit(dev);
	
	if(blockNumber < 0 || blockNumber >= filedisk.nBlocks)
	{
		T(YAFFS_TRACE_ALWAYS,("Attempt to erase non-existant block %d\n",blockNumber));
		return YAFFS_FAIL;
	}
	else
	{
	
		yflash_Page pg;
		
		memset(&pg,0xff,sizeof(yflash_Page));
		
		lseek(filedisk.handle, blockNumber * 32 * 528, SEEK_SET);
		
		for(i = 0; i < 32; i++)
		{
			write(filedisk.handle,&pg,528);
		}
		return YAFFS_OK;
	}
	
}

int yflash_InitialiseNAND(yaffs_Device *dev)
{
	dev->useNANDECC = 1; // force on useNANDECC which gets faked. 
						 // This saves us doing ECC checks.
	
	return YAFFS_OK;
}


